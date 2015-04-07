/*
 * pg12864.c
 *
 * Implements an LCD interface - PG 12864-N Powertip module using the AT91EB40A
 * board
 *
 * Interface details
 * =================
 *
 * The PG 12864-N is driven using its serial port - SCL and SDA.  Four
 * additional control signals are employed - *RES, *CS, RS and P/S, details
 * as follows:
 *
 *   Signal
 *  ================================================
 *    SDA   Serial data input signal
 *    SCL   Serial data transfer clock
 *    *RES  Reset
 *    *CS   Chip select
 *    RS    Identify data sent
 *    P/S   Switch between parallel and serial ports
 *  =================================================
 *
 * Power to drive the display is derived directly from the EB40A board,
 * nominally 3V3.
 *
 *
 *  PG12864 pin   AT91 Port Pin   EB40A P2 Pin
 *  ==========================================
 *    2 - *RES          P16        A16
 *    3 - *CS           P17        A18
 *    4 - RS            P18        A19
 *    5 - P/S           P19        A20
 *   17 - SCL           P3/TCLK1   B4
 *   18 - SDA           P4/TIOA1   B5
 *
 *    1 - Vss                      A1 - GND
 *   16 - Vdd                      A7 - VCC3V3
 * ===========================================
 *
 * June, 2005 - WDH
 *   Initial work
 * July, 2006 - WDH
 *   Corrected VRAM model
 *
 *
 * Informed by published code of Michael Beigl, TecO, University of Karlsruhe
 *
 *
 * PG 12864-N Coordinates
 *
 *    .--x->--------------------------
 *    |........ [x=0,y=0]           |
 *    |                             |
 *    y                             |
 *    |                             |   Origin is Top Left corner
 *    V                             |
 *    |                             |
 *    ~                             ~
 *    |                             |
 *    |                             |
 *    |                             |
 *    |                             |
 *    |                     ........| [x=15,y=63]
 *    -------------------------------
 *
 * The addressable unit is an 8-pixel with an address (x,y) where x in {0..15}
 * and y in {0..63}.  The 8-pixels are arranged horizontally.
 * There are therefore 15*8=128 pixels in the x direction and 64 pixels in the
 * y direction.
 *
 * The PG12864 module incoroporates a LH155BA LCD graphics controller
 *
 */

#include "config.h"
#include "LCDFont.h"
#include "pg12864.h"
#include "AT91PIO.h"
#include "Delay.h"


/* Delay between serial bits */

#define WDELAY 1


/* Global x and y variables for display position */
static char LCD_x_global;
static char LCD_y_global;

/*
 * Display RAM - arranged in 16 bytes per row and 64 rows
 * VRAM[0][0] is positioned top-left and
 * VRAM[LCD_X_MAX][LCD_Y_MAX] at bottom-right
 * Each VRAM byte contains 8-pixels
 */
static unsigned char VRAM[LCD_X_MAX + 1][LCD_Y_MAX + 1];


/*
 * LCD_Init()
 *
 * Initialise LH155BA Display, power it on etc. and the Video RAM
 *
 * An 8-bit LH155DA command is formatted in 2 fields, as follows:
 *
 *    Bit: 7  6  5  4   3  2  1  0
 *         ----------   ----------
 *           Command      Data
 *           code         code
 *
 * Additionally, the RS bit indicates the following:
 *
 *     RS = 0 : RAM data access - writes to display
 *     RS = 1 : Control register access
 *
 * WDH, June, 2005
 *
 */
void LCD_Init() {

   OutputHigh( CS );           /* Chip not selected */
   OutputLow( RES );           /* Reset device */
   OutputLow( PS );            /* Select Serial interface */
                               /* Note: - could pull PS low at the interface
                                  so save a bit of I/O */
   OutputLow( SCL );           /* Clear serial clock line */
   OutputLow( SDA );           /* Clear serial data line */
   OutputLow( RS );            /* Selects RAM data access */

   Delay_ms( 10 );             /* Delay for power to stabilize */

   OutputHigh(RES);            /* Out of RESET */

   Delay_ms(1);

/* Configure the controller */

/* Power Control Register(1) (1011) - Issue INIT using ACL */

   LCD_WriteByte( 0xb1, 1 );
   Delay_ms(1);
/* Power Control Register(1) (1011) - Deselect ACL and turn power on
   via PON */

   LCD_WriteByte( 0xb2, 1 );

/* Display Control(1) Register (1000) - Display on via ON/OFF */

   LCD_WriteByte( 0x81, 1 );
   Delay_ms(1);

/* Power Control Register(2) (1101) - Set contrast level to 12 (1100)*/

   LCD_WriteByte( 0xdc, 1 );

/* Display Control(2) register (1001) - SWAP mode on and REF on */

   LCD_WriteByte( 0x93, 1 );	/* Display swap lets X begin in upper
                                   left corner default is upper left corner */

/* Increment Control Register (1010) - Set increment mode - increment y */

   LCD_WriteByte( 0xa2, 1 ); 	/* Increment control register; increment
                                   y only */
/* Initialise Video RAM */
   LCD_ClearVRAM();
}


/* LCD_ClearVRAM()
 *
 * Clears video RAM - assigns 0x00 to all bytes
 *
 */
static void LCD_ClearVRAM() {
   unsigned int row;           /* Scans over rows */
   unsigned int col;           /* Scans over columns */

   for ( row= 0; row <= LCD_Y_MAX; row++ )
      for ( col= 0; col <= LCD_X_MAX; col++ )
         VRAM[col][row]= 0x00;
}


/*
 * void LCD_WriteByte( unsigned char c, int data_command)
 *
 * Send a command or data byte to the LH155BA
 *
 * First parameter is the byte to be serially output
 * Second paramerter indicates if this byte is a command or data
 *
 *    Data:         type= 0
 *    Command:      type= 1
 *    Clear screen: type= 3
 *
 * Document: Powertip Tech. Corp. NO.PG12864LRF-NRA-H
 * specifies the serial interface protocol used here.
 *
 */
static void LCD_WriteByte( unsigned char c, unsigned char type ) {

   unsigned char bit;        /* Used to index through bits in the data */

   OutputLow( CS );	     /* activate controller chip */
                             /* CS was deactivated to improve noise immunity */
   OutputLow( SCL );         /* Prepare for clock signal cycle */


/* Output appropriate SR signal, high for command and low for data */

   if ( type == 1 )
      OutputHigh( RS );        /* Command - assert RS */
   else
      OutputLow( RS );	       /* Data - clear RS */


/* Transmit byte from highest bit to lowest bit */

   for ( bit= 0x80 ; bit > 0 ; bit = bit >> 1 ) {

      OutputLow( SCL );

      if ( c & bit )
         OutputHigh( SDA );    /* Data bit is high */
      else
         OutputLow( SDA );     /* Data bit is low */

      Delay_us( WDELAY );

      OutputHigh( SCL );       /* When SCL goes high, data on PIN SDA
                                  are accepted by controller */
      Delay_us( WDELAY );

   }

   OutputLow( SCL ); 	       /* prepare for next byte */
   OutputLow( SDA );

   OutputHigh( CS );	       /* deactive controller chip to avoid noise
                                  on line affecting the LCD */

/* set new x and y coordinates if DATA (not command nor clear screen */

   if ( type == 0 ) {                   /* Is this a data command? */
      if ( LCD_y_global < LCD_Y_MAX )   /* End of a row? */
         LCD_y_global++;                /* No - increment y position */
      else
         LCD_y_global= 0;               /* Yes - start next row */
   }

}


/* LCD_Home();
 *
 * Set text position to upper left corner [0,0]
 *
 * Sets both the X and Y Address Registers and assigns the global
 * X and Y cordinates
 *
 */
void LCD_Home() {

/* Setup X and Y Address Registers */

   LCD_WriteByte( 0x00, 1 );      /* Command= 0000, x = 0 */
   LCD_WriteByte( 0x20, 1 );      /* Command= 0010, y a0= a1= a2= a3= 0 */
   LCD_WriteByte( 0x30, 1 );      /* Command= 0011, y a6= a5= a4= 0 */

   LCD_x_global = 0;               /* Initialize coords */
   LCD_y_global = 0;
}




/* LCD_y_pos()
 *
 * Position the cursor - y position
 *
 * y in {0..LCD_Y_MAX}
 *
 * Two commands are required:
 *    1: Assign lowest 4-bits, AY3 .. AY0
 *    2: Assign high 3-bits, AY6..AY4
 *
 * A y address may take values from 0 to 63 in the graphics RAM,
 * the values 64..66 address Segment Display RAM.  The function
 * rejects addresses in Segment Display RAM.
 *
 * Sets the Y Address Register and the global Y position coordinate
 *
 */
void LCD_y_pos( char y ) {

/* Check parameters */
   if ( y > LCD_Y_MAX )
      return;

   LCD_y_global = y;           /* Assign global coordinate */

/* Command= 0010 AY3 AY2 AY1 AY0 */

   LCD_WriteByte( 0x20 | ( 0x0F & y ), 1 );

/* Command= 0011 X AY6 AY5 AY4 */

   LCD_WriteByte( ( ( 0x70 & y ) >> 4 ) | 0x30, 1 );

}




/* LCD_x_pos()
 *
 * Position the cursor - x position
 *
 * x in {0..LCD_X_MAX}
 *
 * An X address may take values from 0 to 15 in the graphics RAM
 * Sets the X Address Register and the global X position coordinate
 *
 */
void LCD_x_pos( unsigned char x) {

/* Check parameters */

   if ( x > LCD_X_MAX )
      return;

/* Reset global x coordinate */

   LCD_x_global = x;

/* Assign X address in controller, command= 0000XXXX */

   LCD_WriteByte( 0x0f & x, 1 );

}

/*
 * Public function to position the cursor
 * x and y are character coordinates [0..LCD_X_MAX, 0..LCD_Y_MAX]
 * y refers to pixel coordinates - so characters can be positioned
 * vertically to pixel positions.
 */
void LCD_PositionCursor( unsigned char x, unsigned char y ) {

/* Check parameters */
   if ( ( x > LCD_X_MAX ) || ( y > LCD_Y_MAX ) )
      return;

   LCD_x_pos( x );
   LCD_y_pos( y );

}



/* LCD_ClearDisplay()
 *
 * Clear the LH155BA display
 *
 * Writes the byte 0x00 to every RAM address from [0,0] to [15,63]
 * Uses the LH155BA automatic X and Y address increment operation
 * for speed.
 * Also clears VRAM data
 *
 */
void LCD_ClearDisplay() {

   int i;

/* Set autoincrement on X and Y */

   LCD_WriteByte( 0xa3, 1 );   /* Increment control register; increment
                                  x+y=0011 */

/* Start at top left corner */

   LCD_WriteByte( 0x00, 1 );   /* x = 0 */
   LCD_WriteByte( 0x20, 1 );   /* y a0, a1, a2, a3 = 0 */
   LCD_WriteByte( 0x30, 1 );   /* y a6, a5, a4 */

/* Write to every RAM address */

   for (i=0; i <= ( LCD_X_MAX + 1 ) * ( LCD_Y_MAX + 1 ); i++ )
      LCD_WriteByte( 0, 3 );

/* Put autoincrement back to Y only */

   LCD_WriteByte( 0xa2, 1 );   /* Increment control register;
                                  increment y */

/* Put cursor at home position and global coordinates also */
   LCD_Home();

/* Clear VRAM */
   LCD_ClearVRAM();

}



/*
 * LCD_SetPixel( X, Y )
 *
 * Sets a pixel at a given address
 * corresponding to the point [X,Y]
 *
 * X in {0..( LCD_X_MAX * 8 ) + 7}
 * Y in {0..LCD_Y_MAX}
 *
 * Sets the pixel in VRAM and then writes the byte containing that pixel
 * to the LCD
 *
 * If the pixel address is out of range, the function does nothing
 *
 */
void LCD_SetPixel( unsigned char x, unsigned char y ) {

/* Test coordinates in VRAM address range */

  if ( ( x > ( LCD_X_MAX * 8 + 7 ) ) || ( y > LCD_Y_MAX ) )
     return;

/* Logical OR this pixel with remaining pixels in this byte */

  VRAM[ x / 8 ][ y ] |= 0x80 >> ( x % 8 );

/* Position cursor */

  LCD_x_pos( x / 8 );
  LCD_y_pos( y );

/* Update the display */

  LCD_WriteByte( VRAM[ x / 8 ][ y ], 0 );

}


/*
 * LCD_ClearPixel( X, Y )
 *
 * Clears a pixel at a given address
 * corresponding to the point [X,Y]
 *
 * X in {0..( LCD_X_MAX * 8 ) + 7}
 * Y in {0..LCD_Y_MAX}
 *
 * Clears the pixel in VRAM and then writes the byte containing that pixel
 * to the LCD
 *
 * If the pixel address is out of range, the function does nothing
 *
 */
void LCD_ClearPixel( unsigned char x, unsigned char y ) {

/* Test coordinates in VRAM address range */

  if ( ( x > ( LCD_X_MAX * 8 + 7 ) ) || ( y > LCD_Y_MAX ) )
     return;

/* Logical AND this pixel with remaining pixels in this byte */

  VRAM[ x / 8 ][ y ] &= ~(0x80 >> ( x % 8 ));

/* Position cursor */

  LCD_x_pos( x / 8 );
  LCD_y_pos( y );

/* Update the display */

  LCD_WriteByte( VRAM[ x / 8 ][ y ], 0 );

}



/*
 * Print a single character on the LCD at the current cursor position
 */
void LCD_PutChar(char c) {
   unsigned char i;
   unsigned char fontIndex;
   unsigned char y;

/* Handle line feed character */

   if ( c == '\n' ) {
      LCD_x_pos(0);                          /* Start of line */
      if ( LCD_y_global + 9 < LCD_Y_MAX )    /* End of display? */
         LCD_y_pos( LCD_y_global + 8 );      /* No - start another line */
      else
         LCD_y_pos ( 0 );                    /* Yes - start at top */

      return;
   }

/* Handle other characters */

   if ( ( c < 32 ) || ( c > 223 ) )   /* Is this a renderable character? */
      return;

   fontIndex = ( c - 32 ) % 32;       /* Compute index within a font table */
   y = LCD_y_global;

/* Get font data and output to display */
/* Note: does not update VRAM! */

   for ( i = 0; i < 8; i++ ) {
      switch ( ( c - 32 ) / 32 ) {
	 case 0: LCD_WriteByte( _LCD1_1_FONT[fontIndex].b[i], 0 ); break;
	 case 1: LCD_WriteByte( _LCD1_2_FONT[fontIndex].b[i], 0 ); break;
	 case 2: LCD_WriteByte( _LCD1_3_FONT[fontIndex].b[i], 0 ); break;
	 case 3: LCD_WriteByte( _LCD1_4_FONT[fontIndex].b[i], 0 ); break;
	 case 4: LCD_WriteByte( _LCD1_5_FONT[fontIndex].b[i], 0 ); break;
	 case 5: LCD_WriteByte( _LCD1_6_FONT[fontIndex].b[i], 0 ); break;
	 case 6: LCD_WriteByte( _LCD1_7_FONT[fontIndex].b[i], 0 ); break;
      }
   }

/* Adjust cursor position */

   if ( LCD_x_global < LCD_X_MAX ) {
      LCD_y_pos( y );
      LCD_x_pos( LCD_x_global + 1 );
   } else {
      if ( LCD_y_global == 0 )          /* right downmost position ! */
         LCD_Home();                    /* Home position */
      else                              /* line feed */
         LCD_x_pos( 0 );
   }

}


/*
 * Print a string (ending with a NULL)
 */
void LCD_PutString( char * string ) {
  char i= 0;

  while (string[i] != 0 ) {
     LCD_PutChar( string[i] );
     i++;
  }

}
