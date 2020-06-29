/* WARNING: this is still at the work-in-progres proof-of-concept stage.
 * BigVGA library and command-line tool to set high-resolution text modes in
 * FreeDOS and other DOS-compatible systems.
 *
 * For more information see README.md.
 *
 * The latest version of BigVGA is available at:
 * https://github.com/LinoMastro/bigvga
 */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

/* Addresses in the BIOS Data Area, relative to segment 0. */
#define BDA_COLUMNS         0x44A       /* 16-bit word */
#define BDA_FRAME_SIZE      0x44C       /* 16-bit word */
#define BDA_CRTC_PORT       0x463       /* word, VGA CRT Controller I/O port */
#define BDA_LINES_MINUS_ONE 0x484       /* byte */

/* VGA CRT registers. */
#define VGA_CRTC_NUM_REGISTERS 25       /* Number of registers of the VGA CRTC. */
#define VGA_CRTC_H_TOTAL       0
#define VGA_CRTC_H_DISP        1
#define VGA_CRTC_H_BLANK_START 2
#define VGA_CRTC_H_BLANK_END   3
#define VGA_CRTC_H_SYNC_START  4
#define VGA_CRTC_H_SYNC_END    5
#define VGA_CRTC_V_TOTAL       6
#define VGA_CRTC_OVERFLOW      7
#define VGA_CRTC_PRESET_ROW    8
#define VGA_CRTC_MAX_SCAN      9
#define VGA_CRTC_CURSOR_START  0x0A
#define VGA_CRTC_CURSOR_END    0x0B
#define VGA_CRTC_START_HI      0x0C
#define VGA_CRTC_START_LO      0x0D
#define VGA_CRTC_CURSOR_HI     0x0E
#define VGA_CRTC_CURSOR_LO     0x0F
#define VGA_CRTC_V_SYNC_START  0x10
#define VGA_CRTC_V_SYNC_END    0x11
#define VGA_CRTC_V_DISP_END    0x12
#define VGA_CRTC_OFFSET        0x13
#define VGA_CRTC_UNDERLINE     0x14
#define VGA_CRTC_V_BLANK_START 0x15
#define VGA_CRTC_V_BLANK_END   0x16
#define VGA_CRTC_MODE          0x17
#define VGA_CRTC_LINE_COMPARE  0x18

/* Various constants used to calculate timings, the exact values are not
 * important on emulators.
 *
 * On classic CRT screens the horizontal back porch is the width of the border
 * to the left of the active screen area, the horizontal front porch is the
 * border to the right of the active area.
 *
 * Similarly the vertical back porch is the border above the active area and
 * the vertical front porch is the border below the screen.
 *
 * Horizontal and vertical sync are used to signal the start of a new scanline
 * and entire screen refresh respectively.
 *
 * All vertical values are in pixel, while the horizontal values are in
 * multiples of the character width (usually 9 pixels).
 */
#define H_FRONT_PORCH 5
#define H_SYNC        12
#define H_BACK_PORCH  3
#define V_FRONT_PORCH 12
#define V_SYNC        2
#define V_BACK_PORCH  35

/* Font height in pixel. Must match the BIOS default text font. */
#define FONT_HEIGHT 16

/* A series of hacks to correctly compile on older C compilers:
 * Borland Turbo C, Microsoft Quick C and Bruce C compiler (bcc).
 *
 * Modern compilers like gcc-ia16, OpenWatcom and DJGPP should work out of the
 * box.
 */
#ifdef __TURBOC__
#define __far far
#define _disable disable
#define _enable enable
#endif

#ifdef _QC
#define __far far
#define MK_FP(seg, off) ((void far *)(((long)(seg) << 16) | (unsigned)(off)))
#endif

#ifdef __BCC__
void
_disable(void)
{
    asm("cli");
}

void
_enable(void)
{
    asm("sti");
}

int
inp(unsigned port)
{
    asm("mov bx, sp");
    asm("mov dx, [bx+2]");
    asm("in al, dx");
    asm("mov ah, #0");
}

void
outp(unsigned port, int value)
{
    asm("mov bx, sp");
    asm("mov dx, [bx+2]");
    asm("mov al, [bx+4]");
    asm("out dx, al");
}
#endif

/* Read a byte from the given segment and offset in memory. */
unsigned char
read_byte(unsigned seg, unsigned off)
{
#ifdef __BCC__
    unsigned char c = 0;
    movedata(seg, off, __get_ds(), &c, 1);
    return c;
#else
    unsigned char __far *p = MK_FP(seg, off);
    return *p;
#endif
}

/* Write a byte to the given segment and offset in memory. */
void
write_byte(unsigned seg, unsigned off, unsigned char value)
{
#ifdef __BCC__
    movedata(__get_ds(), &value, seg, off, 1);
#else
    unsigned char __far *p = MK_FP(seg, off);
    *p = value;
#endif
}

/* Read a 16-bit word from the given segment and offset in memory. */
unsigned
read_word(unsigned seg, unsigned off)
{
    return read_byte(seg, off) | ((unsigned)read_byte(seg, off + 1) << 8);
}

/* Write a 16-bit word to the given segment and offset in memory. */
void
write_word(unsigned seg, unsigned off, unsigned value)
{
    write_byte(seg, off, (unsigned char)(value & 0xff));
    write_byte(seg, off + 1, (unsigned char)(value >> 8));
}

/* Read a VGA register from a given port. */
unsigned char
read_vga(unsigned port, int reg)
{
    unsigned char value;
    _disable();
    outp(port, reg);
    value = (unsigned char)inp(port + 1);
    _enable();
    return value;
}

/* Write to a VGA register at a given port. */
void
write_vga(unsigned port, int reg, int value)
{
    _disable();
    outp(port, reg);
    outp(port + 1, value);
    _enable();
}

void
set_mode3(void)
{
    union REGS regs;
    regs.x.ax = 3;
    int86(0x10, &regs, &regs);
}

void
read_crtc_registers(unsigned char *crtc_regs)
{
    unsigned vga_crt = read_word(0, BDA_CRTC_PORT);
    int i;

    for (i = 0; i < VGA_CRTC_NUM_REGISTERS; i++) {
        crtc_regs[i] = read_vga(vga_crt, i);
    }
}

void
write_crtc_registers(unsigned char *crtc_regs)
{
    unsigned vga_crt = read_word(0, BDA_CRTC_PORT);
    int i;
    unsigned char value;

    /* FIXME: should disable interrupts and blank display before messing with CRTC registers */
    value = crtc_regs[VGA_CRTC_V_SYNC_END];
    value &= 0x7f;      /* Disable register write protection. */
    write_vga(vga_crt, VGA_CRTC_V_SYNC_END, value);

    for (i = 0; i < VGA_CRTC_NUM_REGISTERS; i++) {
        write_vga(vga_crt, i, crtc_regs[i]);
    }
}

/* Print the current values of all VGA CRT Controller registers. */
void
dump_vga_state(void)
{
    unsigned char crtc_regs[VGA_CRTC_NUM_REGISTERS];
    int i;

    read_crtc_registers(crtc_regs);
    puts("VGA CRT Controller register values");
    for (i = 0; i < VGA_CRTC_NUM_REGISTERS; i++) {
        printf("%02x %02x\n", i, crtc_regs[i]);
    }
}

/* Set a text mode with an arbitrary number of columns and lines. */
int
set_text_mode(unsigned columns, unsigned lines, int dry_run)
{
    unsigned char crtc_regs[VGA_CRTC_NUM_REGISTERS];
    unsigned char value;
    unsigned height_minus_one;

    /* FIXME: better validate the inputs! */
    /* Columns: must be an even number, maximum value is 256 but some software
     * assumes it fits in a single byte.
     * Lines: the maximum value is 256 in theory but max vertical resolution
     * is 1024 pixel and that includes vertical sync and front/back porch.
     */
    if (columns < 40 || columns > 254 || columns % 2 != 0) {
        return 1;
    }
    if (lines < 12 || lines > 60) {
        return 1;
    }
    /* Screen height minus one, maximum is 1024-1 */
    height_minus_one = lines * FONT_HEIGHT - 1;

    if (!dry_run) {
        set_mode3();    /* Let the BIOS do the heavy lifting. */
    }
    read_crtc_registers(crtc_regs);

    /* Set the number of columns. */
    crtc_regs[VGA_CRTC_H_DISP] = (unsigned char)(columns - 1);
    crtc_regs[VGA_CRTC_OFFSET] = (unsigned char)(columns / 2);

    /* Set the number of lines. */
    value = (unsigned char)(height_minus_one & 0xff);    /* Lower 8 bits. */
    crtc_regs[VGA_CRTC_V_DISP_END] = value;
    value = crtc_regs[VGA_CRTC_OVERFLOW];
    value &= 0xbd;      /* Set bits #1 and #6 to 0. */
    /* Copy the top two bits (bit #8 and #9) of the screen height into bits #1
     * and #6 of the register. */
    value |=
        ((height_minus_one >> 8) & 1) * 2 +
        ((height_minus_one >> 9) & 1) * 0x40;
    crtc_regs[VGA_CRTC_OVERFLOW] = value;

    if (!dry_run) {
        /* Reprogram the VGA CRT Controller with the new text mode size. */
        write_crtc_registers(crtc_regs);

        /* Update the BIOS data area to keep in sync with the new settings. */
        write_word(0, BDA_COLUMNS, columns);
        write_word(0, BDA_FRAME_SIZE, columns * lines * 2);     /* FIXME: this should probably be a divisor of 32kB */
        write_byte(0, BDA_LINES_MINUS_ONE, (unsigned char)(lines - 1));
    }
    return 0;
}

void
print_help(FILE *fp)
{
    fputs("Usage: FIXME\n", fp);
}

int
main(int argc, char **argv)
{
    int flag_dump_state = 0, flag_help = 0, flag_unknown = 0;
    int flag_dry_run = 0, flag_verbose = 0;
    unsigned columns = 80, lines = 25;
    int i;

    for (i = 1; i < argc; i++) {
        if ((argv[i][0] == '/' || argv[i][0] == '-') && argv[i][2] == '\0') {
            switch (argv[i][1]) {
                case '?':
                case 'h':
                    flag_help = 1;
                    break;
                case 'd':
                    flag_dump_state = 1;
                    break;
                case 'n':
                    flag_dry_run = 1;
                    break;
                case 'v':
                    flag_verbose = 1;
                    break;
                default:
                    flag_unknown = 1;
                    break;
            }
        } else {
            /* Parse an argument in the form NNNxNNN. */
            if (sscanf(argv[i], "%ux%u", &columns, &lines) != 2) {
                flag_unknown = 1;
            }
        }
    }
    if (flag_unknown) {
        fputs("Error: invalid command arguments.\n\n", stderr);
        print_help(stderr);
        return 1;
    }
    if (flag_help || argc == 1) {
        print_help(stdout);
        return 1;
    }
    if (flag_dump_state) {
        dump_vga_state();
        return 0;
    }
    printf("Setting text mode %ux%u\n", columns, lines);
    if (set_text_mode(columns, lines, flag_dry_run)) {
        fputs("Error: invalid number of columns or lines.\n\n", stderr);
        print_help(stderr);
        return 1;
    } else {
        printf("Text mode %ux%u\n", columns, lines);
    }
    return 0;
}
