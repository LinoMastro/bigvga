BigVGA
======

BigVGA is a library and a command-line tool to set high-resolution text modes
in FreeDOS and other DOS-compatible systems.

WARNING: this is still at the work-in-progres proof-of-concept stage but basic
functionality should already work in QEMU. Try e.g. `bigvga.exe 136x43` to set
a text mode with 136 columns and 43 lines (and approximate 16:9 aspect ratio)
in QEMU. Revert to a normal 80x25 text mode by running the DOS command
`mode co80` or rebooting your system.

Unlike other projects that rely on SuperVGA hardware, BigVGA only uses basic
VGA features.

The main target is emulators (e.g. qemu, dosbox, dosemu, bochs...) but it might
also work on some laptops or LCD screens.

Help, my screen is garbled or blank
-----------------------------------

If after running BigVGA you want to go back to a safe screen state, optionally
press CTRL-C to make sure you are at the DOS prompt and then type "mode co80"
(without the quotes) and press Enter. You can type this command even blindly if
you don't see the output if the screen is unreadable.

If that doesn't help, simply reboot your emulator or OS: BigVGA doesn't make
any permanent change, all changes are reverted at system reboot.

Emulator-specific details
-------------------------

QEMU by default resizes its window in response to screen resize requests from
BigVGA, however if you manually change the QEMU window size all subsequent
requests from BigVGA will only change the state inside the emulator and QEMU
will scale the text to fit into the window size that you previously set. A
downside of this scaling is that text will appear blurry. To switch back the
QEMU window to the "native" unscaled dimensions press CTRL-ALT-U.

Download
--------

BigVGA is free software, released under the MIT License. See the LICENSE file
for details.

The latest version of BigVGA is available at:
https://github.com/LinoMastro/bigvga

Compatibility
-------------

BigVGA should be compatible with a wide range of DOS compilers and
DOS-compatible operating systems, including FreeDOS.

It should compile correctly and without warnings in gcc-ia16 (with the i86
library), Open Watcom, DJGPP, Borland Turbo C, Microsoft Quick C and Bruce C
compiler (bcc, with the `-ansi` flag).

BigVGA text modes are identical to the standard BIOS text mode 3 (e.g. same
video memory location at segment 0xB800) except that they have a different
number of columns and/or lines than the usual 80x25.

Most commandline utilities should work fine with non-standard text screen sizes
but fullscreen TUI programs are a bit hit-and-miss. For example the FreeDOS
edit 0.9a program accepts any number of columns but expects the number of lines
to be 25, 43 or 50. The `vi.exe` editor part of Open Watcom instead works in
any BigVGA mode.

Generally any program that outputs text via BIOS or DOS syscalls should work
correctly and even programs that directly access the video memory should be
compatible with BigVGA text modes provided that they don't hardcode the number
of columns and lines to be 80x25 and instead read the actual values from the
BIOS Data Area and/or from a BIOS Int 10/AH=0Fh call.

Low-level programs like mouse drivers and `NANSI.SYS` should also be fully
compatible with BigVGA text modes.

So far most testing has been done in QEMU, compatibility with more emulators
and potentially even some real hardware will improve as BigVGA evolves. Most
emulated VGA adapters should work, BigVGA doesn't require a SuperVGA card.

Dual-graphics-card dual-screen DOS systems (e.g. with a VGA and a MDA adapter)
should be compatible. Make sure that the active card is the VGA one before
running BigVGA.

Since BigVGA modes use a very low refresh rate they will likely never be
compatible with real CRT screens over a VGA connector.

Prior art
---------

The FreeDOS **mode** command can also change text mode resolution (e.g. `mode
con cols=80 lines=28`) however it only uses standard BIOS calls and is limited
to a few available modes. On VGA adapters the "mode" command never exceeds a
resolution of 720x400 pixels for maximum hardware compatibility, so for example
the 80x50 mode is achieved by using a small 9x8 font, while BigVGA uses the
standard VGA 9x16 font and for the same 80x50 mode offers a much bigger screen
size in emulators.

The **wtm** (Windowed Text Mode) program by Jason Hood works in a very similar
way to BigVGA but provides more limited hardware and emulator compatibility.
It's available at: http://adoxa.altervista.org/tm/

The **SVGATextMode** program for Linux and DOS has similar goals but requires
a supported SVGA adapter to go beyond the usual VGA text modes:
http://freshmeat.sourceforge.net/projects/svgatextmode

Contributing
------------

Bug reports, feature requests and PRs are welcome via the BigVGA GitHub page:
https://github.com/LinoMastro/bigvga

Detailed information about standard VGA registers can be found at
http://www.o3one.org/hwdocs/vga/vga_app.html and from the FreeVGA project page
http://www.osdever.net/FreeVGA/home.htm and specifically for the CRT registers
at http://www.osdever.net/FreeVGA/vga/crtcreg.htm

The preferred style and indentation is the one generated by the following GNU
indent command:

    indent -gnu -br -ce -brs -ncs -di4 -i4 -npcs -nut -cli4 -c1 -T FILE *.c *.h
