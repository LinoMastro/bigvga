#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

/* Addresses in the BIOS Data Area, relative to segment 0. */

/* Word in the BDA containing the I/O port of the VGA CRT Controller. */
#define BDA_CRTC_PORT 0x463

#define VGA_CRTC_NUM_REGISTERS 25  /* The VGA CRTC has 25 registers. */

/* VGA CRT registers */
#define VGA_CRTC_H_DISP     1
#define VGA_CRTC_OVERFLOW   7
#define VGA_CRTC_V_SYNC_END 0x11
#define VGA_CRTC_V_DISP_END 0x12
#define VGA_CRTC_OFFSET     0x13

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
void _disable(void) {
    asm("cli");
}

void _enable(void) {
    asm("sti");
}

int inp(unsigned port) {
    asm("mov bx, sp");
    asm("mov dx, [bx+2]");
    asm("in al, dx");
    asm("mov ah, #0");
}

void outp(unsigned port, int value) {
    asm("mov bx, sp");
    asm("mov dx, [bx+2]");
    asm("mov al, [bx+4]");
    asm("out dx, al");
}
#endif

/* Read a byte from the given segment and offset in memory. */
unsigned char read_byte(unsigned seg, unsigned off) {
#ifdef __BCC__
    unsigned char c = 0;
    movedata(seg, off, __get_ds(), &c, 1);
    return c;
#else
    unsigned char __far *p = MK_FP(seg, off);
    return *p;
#endif
}

/* Read a 16-bit word from the given segment and offset in memory. */
unsigned read_word(unsigned seg, unsigned off) {
    return read_byte(seg, off) | ((unsigned)read_byte(seg, off + 1) << 8);
}

/* Read a VGA register from a given port. */
int read_vga(unsigned port, int reg) {
    int value;
    _disable();
    outp(port, reg);
    value = inp(port + 1);
    _enable();
    return value;
}

/* Write to a VGA register at a given port. */
void write_vga(unsigned port, int reg, int value) {
    _disable();
    outp(port, reg);
    outp(port + 1, value);
    _enable();
}

void set_mode3(void) {
    union REGS regs;
    regs.x.ax = 3;
    int86(0x10, &regs, &regs);
}

void read_crtc_registers(unsigned char *crtc_regs) {
    unsigned vga_crt = read_word(0, BDA_CRTC_PORT);
    int i;

    for (i = 0; i < VGA_CRTC_NUM_REGISTERS; i++) {
        crtc_regs[i] = read_vga(vga_crt, i);
    }
}

void dump_vga_state(void) {
    unsigned char crtc_regs[VGA_CRTC_NUM_REGISTERS];
    int i;

    read_crtc_registers(crtc_regs);
    puts("VGA CRT Controller register values");
    for (i = 0; i < VGA_CRTC_NUM_REGISTERS; i++) {
        printf("%02x %02x\n", i, crtc_regs[i]);
    }
}

int main(int argc, char **argv) {
    int flag_dump_state = 0, flag_help = 0, flag_unknown = 0;
    int flag_dry_run = 0, flag_verbose = 0;
    unsigned columns = 80, rows = 25;
    int i;

    for (i = 1; i < argc; i++) {
        printf("\"%s\"\n", argv[i]);
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
            if (sscanf(argv[i], "%ux%u", &columns, &rows) != 2) {
                flag_unknown = 1;
            }
        }
    }
    if (flag_unknown) {
        fputs("Error: invalid command arguments.\n\n", stderr);
    }
    if (flag_unknown || flag_help || argc == 1) {
        fputs("Usage: FIXME\n", stderr);
        return 1;
    }
    if (flag_dump_state) {
        dump_vga_state();
        return 0;
    }
    printf("Setting text mode %ux%u\n", columns, rows);
    return 0;
}
