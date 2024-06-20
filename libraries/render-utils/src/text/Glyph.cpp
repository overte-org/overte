#include "Glyph.h"
#include <StreamHelpers.h>

// We adjust bounds because offset is the bottom left corner of the font but the top left corner of a QRect
QRectF Glyph::bounds() const {
    return glmToRect(offset, size).translated(0.0f, -size.y);
}

QRectF Glyph::textureBounds() const {
    return glmToRect(texOffset, texSize);
}
