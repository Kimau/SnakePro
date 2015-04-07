/* pg12864.h */

/* Device addresses
 * Outputs:
 */
#define RES 0x00010000
#define CS  0x00020000
#define RS  0x00040000
#define PS  0x00080000
#define SCL 0x00000008
#define SDA 0x00000010

/* Maximal screen character coordinate limits */

#define LCD_X_MAX  0x0F
#define LCD_Y_MAX  0x3F

void LCD_Init();
void LCD_Home();
void LCD_ClearDisplay();
void LCD_SetPixel( unsigned char x, unsigned char y );
void LCD_ClearPixel( unsigned char x, unsigned char y );
void LCD_PutString( char * string );
void LCD_PositionCursor( unsigned char x, unsigned char y );
void LCD_PutChar( char c );

void LCD_WriteByte( unsigned char, unsigned char );
void LCD_ClearVRAM( void );
