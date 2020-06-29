/* BigVGA library to set high-resolution text modes in FreeDOS and other
 * DOS-compatible systems.
 *
 * For more information see README.md.
 *
 * The latest version of BigVGA is available at:
 * https://github.com/LinoMastro/bigvga
 */

#ifndef BIGVGA_H_
#define BIGVGA_H_

void set_mode3(void);

/* Print the current values of all VGA CRT Controller registers. */
void dump_vga_state(void);

/* Set a text mode with an arbitrary number of columns and lines. */
int set_text_mode(unsigned columns, unsigned lines, int dry_run);

#endif /* BIGVGA_H_ */
