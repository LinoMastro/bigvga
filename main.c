#include <stdio.h>

#include "bigvga.h"

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
