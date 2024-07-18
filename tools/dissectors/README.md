# High Fidelity Wireshark Plugins


## Installation


* Install wireshark 2.4.6 or higher.
* Copy these lua files into `c:\Users\username\AppData\Roaming\Wireshark\Plugins` on Windows, or `$HOME/.local/lib/wireshark/plugins` on Linux.

## Lua version

This is a Lua plugin, which requires the bit32 module to be installed. You can find the Lua version wireshark uses in the About dialog, eg:

    Version 4.2.5 (Git commit 798e06a0f7be).

    Compiled (64-bit) using GCC 14.1.1 20240507 (Red Hat 14.1.1-1), with GLib
    2.80.2, with Qt 6.7.0, with libpcap, with POSIX capabilities (Linux), with libnl
    3, with zlib 1.3.0.zlib-ng, with PCRE2, with Lua 5.1.5, with GnuTLS 3.8.5 and

This indicates Lua 5.1 is used (see on the last line)


## Requirements

On Fedora 40:

* wireshark-devel
* lua5.1-bit32


## Usage

After a capture any detected Overte Packets should be easily identifiable by one of the following protocols

* `HF-AUDIO` - Streaming audio packets
* `HF-AVATAR` - Streaming avatar mixer packets
* `HF-ENTITY` - Entity server traffic
* `HF-DOMAIN` - Domain server traffic
* `HFUDT` - All other UDP traffic




## Troubleshooting

### attempt to index global 'bit32' (a nil value)

`[Expert Info (Error/Undecoded): Lua Error: /home/dale/.local/lib/wireshark/plugins/1-hfudt.lua:207: attempt to index global 'bit32' (a nil value)]`

See the installation requirements, you need to install the bit32 Lua module for the right Lua version.
