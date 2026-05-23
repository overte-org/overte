#[cxx::bridge(namespace = "render_utils::rust")]
mod ffi {

    // NOTE: This only implements the float variant
    struct Arfont {
        metadata_format: MetadataFormat,
        metadata: String,
        variants: Vec<FontVariant>,
        images: Vec<FontImage>,
        appendices: Vec<FontAppendix>,
    }
    enum ArfontError {
        Ok,
        UnexpectedEOF,
        BrokenMagic,
        MalformedFile,
        MalformedString,
        Unknown,
    }
    #[repr(u32)]
    enum CodepointType {
        CP_UNSPECIFIED = 0,
        CP_UNICODE = 1,
        CP_INDEXED = 2,
        CP_ICONOGRAPHIC = 14,
    }

    #[repr(u32)]
    enum MetadataFormat {
        METADATA_NONE = 0,
        METADATA_PLAINTEXT = 1,
        METADATA_JSON = 2,
    }

    #[repr(u32)]
    enum ImageType {
        IMAGE_NONE = 0,
        IMAGE_SRGB_IMAGE = 1,
        IMAGE_LINEAR_MASK = 2,
        IMAGE_MASKED_SRGB_IMAGE = 3,
        IMAGE_SDF = 4,
        IMAGE_PSDF = 5,
        IMAGE_MSDF = 6,
        IMAGE_MTSDF = 7,
        IMAGE_MIXED_CONTENT = 255,
    }
    #[repr(u32)]
    enum PixelFormat {
        PIXEL_UNKNOWN = 0,
        PIXEL_BOOLEAN1 = 1,
        PIXEL_UNSIGNED8 = 8,
        PIXEL_FLOAT32 = 32,
    }

    #[repr(u32)]
    enum ImageEncoding {
        IMAGE_UNKNOWN_ENCODING = 0,
        IMAGE_RAW_BINARY = 1,
        IMAGE_BMP = 4,
        IMAGE_TIFF = 5,
        IMAGE_PNG = 8,
        IMAGE_TGA = 9,
    }

    #[repr(i32)]
    enum ImageOrientation {
        ORIENTATION_TOP_DOWN = 1,
        ORIENTATION_BOTTOM_UP = -1,
    }

    struct Bounds {
        pub l: f32,
        pub b: f32,
        pub r: f32,
        pub t: f32,
    }
    struct Advance {
        pub h: f32,
        pub v: f32,
    }
    struct Glyph {
        pub codepoint: u32,
        pub image: u32,
        pub plane_bounds: Bounds,
        pub image_bounds: Bounds,
        pub advance: Advance,
    }
    struct KernPair {
        codepoint1: u32,
        codepoint2: u32,
        advance: Advance,
    }
    struct Metrics {
        // In pixels:
        fontSize: f32,
        distanceRange: f32,
        // Proportional to font size:
        emSize: f32,
        ascender: f32,
        descender: f32,
        lineHeight: f32,
        underlineY: f32,
        underlineThickness: f32,
        // In pixels:
        distanceRangeMiddle: f32,
    }
    struct RawBinaryFormat {
        row_length: u32,
        orientation: ImageOrientation,
    }

    struct FontVariant {
        flags: u32,
        weight: u32,
        codepoint_type: CodepointType,
        image_type: ImageType,
        fallback_variant: u32,
        fallback_glyph: u32,
        metrics: Metrics,
        name: String,
        metadata: String,
        glyphs: Vec<Glyph>,
        kern_pairs: Vec<KernPair>,
    }
    struct FontImage {
        flags: u32,
        encoding: ImageEncoding,
        width: u32,
        height: u32,
        channels: u32,
        pixel_format: PixelFormat,
        image_type: ImageType,
        raw_binary_format: RawBinaryFormat,
        child_images: u32,
        texture_flags: u32,
        metadata: String,
        data: Vec<u8>,
    }
    struct FontAppendix {
        metadata: String,
        data: Vec<u8>,
    }

    // Rust types and signatures exposed to C++.
    extern "Rust" {
        fn decode_arfont(input: &CxxVector<u8>, out: &mut Arfont) -> ArfontError;
    }

    // C++ types and signatures exposed to  Rust.
    unsafe extern "C++" {}
}

use cxx::CxxVector;
use std::io::Read;

use crate::ffi::{Arfont, ArfontError};

impl From<std::io::ErrorKind> for ArfontError {
    fn from(v: std::io::ErrorKind) -> Self {
        use std::io::ErrorKind;
        match v {
            ErrorKind::UnexpectedEof => Self::UnexpectedEOF,
            _ => todo!(),
        }
    }
}

impl From<[f32; 32]> for ffi::Metrics {
    fn from(val: [f32; 32]) -> Self {
        ffi::Metrics {
            fontSize: val[0],
            distanceRange: val[1],
            emSize: val[2],
            ascender: val[3],
            descender: val[4],
            lineHeight: val[5],
            underlineY: val[6],
            underlineThickness: val[7],
            distanceRangeMiddle: val[8],
        }
    }
}

impl From<std::io::Error> for ArfontError {
    fn from(v: std::io::Error) -> Self {
        v.into()
    }
}

struct CRC32Reader<R: Read> {
    inner: R,
    crc32: crc32fast::Hasher,
    counter: usize,
}

impl<R: Read> CRC32Reader<R> {
    pub fn new(inner: R) -> Self {
        Self {
            inner,
            crc32: crc32fast::Hasher::new(),
            counter: 0,
        }
    }
    pub fn get_counter(&self) -> usize {
        self.counter
    }
    pub fn read_u32(&mut self) -> std::io::Result<u32> {
        let mut buf = [0u8; 4];
        self.read_exact(&mut buf)?;
        Ok(u32::from_le_bytes(buf))
    }
    pub fn read_f32(&mut self) -> std::io::Result<f32> {
        let mut buf = [0u8; 4];
        self.read_exact(&mut buf)?;
        Ok(f32::from_le_bytes(buf))
    }
    pub fn read_bytes(&mut self, len: usize) -> std::io::Result<Vec<u8>> {
        let mut res = vec![0; len];
        self.read_exact(&mut res)?;
        Ok(res)
    }
    pub fn read_string(&mut self, len: usize) -> Result<String, ffi::ArfontError> {
        if len > 0 {
            let buf = self.read_bytes(len)?;
            self.realign()?;
            String::from_utf8(buf).map_err(|_| ffi::ArfontError::MalformedString)
        } else {
            Ok(String::new())
        }
    }
    pub fn read_bounds(&mut self) -> std::io::Result<ffi::Bounds> {
        let l = self.read_f32()?;
        let b = self.read_f32()?;
        let r = self.read_f32()?;
        let t = self.read_f32()?;
        Ok(ffi::Bounds { l, b, r, t })
    }
    pub fn read_advance(&mut self) -> std::io::Result<ffi::Advance> {
        let h = self.read_f32()?;
        let v = self.read_f32()?;
        Ok(ffi::Advance { h, v })
    }
    pub fn realign(&mut self) -> std::io::Result<()> {
        if self.counter & 0x3 != 0 {
            let len = 0x4 - (self.counter & 0x3);
            let mut a = [0u8; 4];
            self.read_exact(&mut a[0..len])?;
        }
        Ok(())
    }

    pub fn finish(self) -> (u32, R, usize) {
        let res = self.crc32.finalize();
        (res, self.inner, self.counter)
    }
}

impl<R: Read> Read for CRC32Reader<R> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        let len = self.inner.read(buf)?;
        self.crc32.update(&buf[0..len]);
        self.counter += len;
        Ok(len)
    }
}

fn decode_arfont_inner(input: &CxxVector<u8>) -> Result<Arfont, ArfontError> {
    let mut reader = CRC32Reader::new(input.as_slice());

    {
        let mut tag = [0u8; 16];
        reader.read_exact(&mut tag)?;
        if &tag != b"ARTERY/FONT\0\0\0\0\0" {
            return Err(ArfontError::BrokenMagic);
        }
        let magic_no = reader.read_u32()?;
        if magic_no != 0x4d276a5c {
            return Err(ArfontError::BrokenMagic);
        }
    }
    let _version = reader.read_u32()?;
    let _flags = reader.read_u32()?;
    {
        let real_type = reader.read_u32()?;
        if real_type != 0x14 {
            return Err(ArfontError::BrokenMagic);
        }
    }
    for _ in 0..4 {
        let _reserved = reader.read_u32()?;
    }
    let metadata_format = {
        let i = reader.read_u32()?;
        match i {
            0 => ffi::MetadataFormat::METADATA_NONE,
            1 => ffi::MetadataFormat::METADATA_PLAINTEXT,
            2 => ffi::MetadataFormat::METADATA_JSON,
            _ => return Err(ffi::ArfontError::MalformedFile),
        }
    };
    let metadata_length = reader.read_u32()?;

    let variant_count = reader.read_u32()?;
    let variant_length = reader.read_u32()?;
    let image_count = reader.read_u32()?;
    let image_length = reader.read_u32()?;
    let appendix_count = reader.read_u32()?;
    let appendix_length = reader.read_u32()?;
    for _ in 0..8 {
        let _reserved = reader.read_u32()?;
    }
    let metadata = reader.read_string(metadata_length as usize)?;

    let prev_length = reader.get_counter();

    let mut variants: Vec<ffi::FontVariant> = Vec::with_capacity(variant_count as usize);
    for _ in 0..variant_count {
        let flags = reader.read_u32()?;
        let weight = reader.read_u32()?;
        let codepoint_type = {
            let i = reader.read_u32()?;
            match i {
                0 => ffi::CodepointType::CP_UNSPECIFIED,
                1 => ffi::CodepointType::CP_UNICODE,
                2 => ffi::CodepointType::CP_INDEXED,
                14 => ffi::CodepointType::CP_ICONOGRAPHIC,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let image_type = {
            let i = reader.read_u32()?;
            match i {
                0 => ffi::ImageType::IMAGE_NONE,
                1 => ffi::ImageType::IMAGE_SRGB_IMAGE,
                2 => ffi::ImageType::IMAGE_LINEAR_MASK,
                3 => ffi::ImageType::IMAGE_MASKED_SRGB_IMAGE,
                4 => ffi::ImageType::IMAGE_SDF,
                5 => ffi::ImageType::IMAGE_PSDF,
                6 => ffi::ImageType::IMAGE_MSDF,
                7 => ffi::ImageType::IMAGE_MTSDF,
                255 => ffi::ImageType::IMAGE_MIXED_CONTENT,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let fallback_variant = reader.read_u32()?;
        let fallback_glyph = reader.read_u32()?;
        for _ in 0..6 {
            let _reserved = reader.read_u32()?;
        }
        let mut metrics = [0.0; 32];
        for i in 0..32 {
            metrics[i] = reader.read_f32()?;
        }
        let metrics = metrics.into();
        let name_length = reader.read_u32()?;
        let metadata_length = reader.read_u32()?;
        let glyph_count = reader.read_u32()?;
        let kern_pair_count = reader.read_u32()?;

        let name = reader.read_string(name_length as usize)?;
        let metadata = reader.read_string(metadata_length as usize)?;

        let mut glyphs = Vec::with_capacity(glyph_count as usize);
        for _ in 0..glyph_count {
            let codepoint = reader.read_u32()?;
            let image = reader.read_u32()?;
            let plane_bounds = reader.read_bounds()?;
            let image_bounds = reader.read_bounds()?;
            let advance = reader.read_advance()?;
            glyphs.push(ffi::Glyph {
                codepoint,
                image,
                plane_bounds,
                image_bounds,
                advance,
            });
        }

        let mut kern_pairs = Vec::with_capacity(kern_pair_count as usize);
        for _ in 0..kern_pair_count {
            let codepoint1 = reader.read_u32()?;
            let codepoint2 = reader.read_u32()?;
            let advance = reader.read_advance()?;
            kern_pairs.push(ffi::KernPair {
                codepoint1,
                codepoint2,
                advance,
            });
        }
        variants.push(ffi::FontVariant {
            flags,
            weight,
            codepoint_type,
            image_type,
            fallback_variant,
            fallback_glyph,
            metrics,
            name,
            metadata,
            glyphs,
            kern_pairs,
        });
    }

    let total_length = reader.get_counter();

    if total_length - prev_length != variant_length as usize {
        return Err(ffi::ArfontError::MalformedFile);
    }
    let prev_length = total_length;

    let mut images = Vec::with_capacity(image_count as usize);
    for _ in 0..image_count {
        let flags = reader.read_u32()?;
        let encoding = {
            let i = reader.read_u32()?;
            match i {
                0 => ffi::ImageEncoding::IMAGE_UNKNOWN_ENCODING,
                1 => ffi::ImageEncoding::IMAGE_RAW_BINARY,
                4 => ffi::ImageEncoding::IMAGE_BMP,
                5 => ffi::ImageEncoding::IMAGE_TIFF,
                8 => ffi::ImageEncoding::IMAGE_PNG,
                9 => ffi::ImageEncoding::IMAGE_TGA,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let width = reader.read_u32()?;
        let height = reader.read_u32()?;
        let channels = reader.read_u32()?;
        let pixel_format = {
            let i = reader.read_u32()?;
            match i {
                0 => ffi::PixelFormat::PIXEL_UNKNOWN,
                1 => ffi::PixelFormat::PIXEL_BOOLEAN1,
                8 => ffi::PixelFormat::PIXEL_UNSIGNED8,
                32 => ffi::PixelFormat::PIXEL_FLOAT32,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let image_type = {
            let i = reader.read_u32()?;
            match i {
                0 => ffi::ImageType::IMAGE_NONE,
                1 => ffi::ImageType::IMAGE_SRGB_IMAGE,
                2 => ffi::ImageType::IMAGE_LINEAR_MASK,
                3 => ffi::ImageType::IMAGE_MASKED_SRGB_IMAGE,
                4 => ffi::ImageType::IMAGE_SDF,
                5 => ffi::ImageType::IMAGE_PSDF,
                6 => ffi::ImageType::IMAGE_MSDF,
                7 => ffi::ImageType::IMAGE_MTSDF,
                255 => ffi::ImageType::IMAGE_MIXED_CONTENT,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let row_length = reader.read_u32()?;
        let orientation = {
            let i = reader.read_u32()? as i32;
            match i {
                // XXX: This is a fix, bcs some file use 0 for whatever reason
                0 | 1 => ffi::ImageOrientation::ORIENTATION_TOP_DOWN,
                -1 => ffi::ImageOrientation::ORIENTATION_BOTTOM_UP,
                _ => return Err(ffi::ArfontError::MalformedFile),
            }
        };
        let child_images = reader.read_u32()?;
        let texture_flags = reader.read_u32()?;
        let _ = reader.read_u32()?;
        let _ = reader.read_u32()?;
        let _ = reader.read_u32()?;
        let metadata_length = reader.read_u32()?;
        let data_length = reader.read_u32()?;
        let metadata = reader.read_string(metadata_length as usize)?;
        let data = reader.read_bytes(data_length as usize)?;
        reader.realign()?;
        images.push(ffi::FontImage {
            flags,
            encoding,
            width,
            height,
            channels,
            pixel_format,
            image_type,
            raw_binary_format: ffi::RawBinaryFormat {
                row_length,
                orientation,
            },
            child_images,
            texture_flags,
            metadata,
            data,
        });
    }

    let total_length = reader.get_counter();

    if total_length - prev_length != image_length as usize {
        return Err(ffi::ArfontError::MalformedFile);
    }
    let prev_length = total_length;

    let mut appendices = Vec::with_capacity(appendix_count as usize);
    for _ in 0..appendix_count {
        let metadata_length = reader.read_u32()?;
        let data_length = reader.read_u32()?;
        let metadata = reader.read_string(metadata_length as usize)?;
        let data = reader.read_bytes(data_length as usize)?;
        appendices.push(ffi::FontAppendix { metadata, data });
    }

    let total_length = reader.get_counter();

    if total_length - prev_length != appendix_length as usize {
        return Err(ffi::ArfontError::MalformedFile);
    }

    {
        let _salt = reader.read_u32()?;
        let magic_no = reader.read_u32()?;
        if magic_no != 0x55ccb363 {
            return Err(ffi::ArfontError::BrokenMagic);
        }
        let _reserved = reader.read_u32()?;
        let _reserved = reader.read_u32()?;
        let _reserved = reader.read_u32()?;
        let _reserved = reader.read_u32()?;
        let total_length = reader.read_u32()?;
        let (checksum, mut reader, final_counter) = reader.finish();
        let mut a = [0u8; 4];
        reader.read_exact(&mut a)?;
        let cs2 = u32::from_le_bytes(a);
        if !checksum != cs2 || total_length as usize != final_counter + 4 {
            println!("ASDFASDF crc32 {:x} {:x}", checksum, cs2);
            return Err(ffi::ArfontError::MalformedFile);
        }
    }

    Ok(ffi::Arfont {
        metadata_format,
        metadata,
        variants,
        images,
        appendices,
    })
}

fn decode_arfont(input: &CxxVector<u8>, out: &mut Arfont) -> ArfontError {
    match decode_arfont_inner(input) {
        Ok(res) => {
            *out = res;
            ArfontError::Ok
        }
        Err(e) => e,
    }
}
