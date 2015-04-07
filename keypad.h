/* keypad.h */

/* Device addresses:
 * 
 * Column lines - X1 .. X4 - left-to-right
 * Row lines    - Y1 .. Y4 - top-to-bottom
 *
 */

/* Columns */
#define X1 0x00001000  /* P12 */
#define X2 0x00000200  /* P9 */
#define X3 0x00000002  /* P1 */
#define X4 0x00000004  /* P2 */

/* Rows */
#define Y1 0x00100000 /* P20 */
#define Y2 0x00200000 /* P21 */
#define Y3 0x00000100 /* P8 */
#define Y4 0x00800000 /* P23 */


int keyPress( void );
