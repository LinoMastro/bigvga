BigVGA
======

WARNING: this is still at the work-in-progres proof-of-concept stage but basic
functionality should already work in QEMU. Try e.g. "bigvga.exe 136x43" to set
a text mode with 136 columns and 43 lines (and approximate 16:9 aspect ratio)
in QEMU. Revert to a normal 80x25 text mode by running the DOS command
"mode co80" or rebooting your system.

BigVGA library and command-line tool to set high-resolution text modes in
FreeDOS and other DOS-compatible systems.

Unlike other projects that rely on SuperVGA hardware, BigVGA only uses basic
VGA features.

The main target is emulators (e.g. qemu, dosbox, dosemu, bochs...) but it might
also work on some laptops or LCD screens.

BigVGA is free software, released under the MIT License. See the LICENSE file
for details.

The latest version of BigVGA is available at:
https://github.com/LinoMastro/bigvga
