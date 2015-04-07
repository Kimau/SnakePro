/* keypad.c
 *
 * Keypad driver
 * Provides a single function:
 *     int keyPress();
 * Returns -1 if no key pressed
 *         and 0x00..0x0F to identify
 *         a key thas is pressed
 *
 * Original - WDH September 2006
 * Updated (improved documentation) - WDH September 2008
 *
 */

#include "config.h"
#include "keypad.h"
#include "Delay.h"
#include "AT91PIO.h"
#include "Sound.h"

int keyCode[4][4] =
{
   {1,2,3,0x0f},
   {4,5,6,0x0e},
   {7,8,9,0x0d},
   {0x0a,0x00,0x0b,0x0c}
};

static void PullRowsLow( void ){
   OutputLow( Y1 );
   OutputLow( Y2 );
   OutputLow( Y3 );   /* In error */
   OutputLow( Y4 );
}

/*
 * Check each column - the column line with a keypress should be low
 */
static int calculateColumn( unsigned long port ) {
   if ( !(port & X1) )
      return 1;
   else if ( !(port & X2) )
      return 2;
   else if ( !(port & X3) )
      return 3;
   else
      return 4;
}


int calculateRow( unsigned long column ) {

   OutputLow(Y1);
   OutputHigh(Y2);
   OutputHigh(Y3);
   OutputHigh(Y4);

   if ( column & ~__PIO_PDSR )
      return 1;
      
   OutputHigh(Y1);
   OutputLow(Y2);

   if ( column & ~__PIO_PDSR )
      return 2;
      
   OutputHigh(Y2);
   OutputLow(Y3);

   if ( column & ~__PIO_PDSR )
      return 3;

   OutputHigh(Y3);
   OutputLow(Y4);

   if ( column & ~__PIO_PDSR )
      return 4;
   
   return 5;
}



/* keyPress()
 *   Returns -1 if no key pressed
 *           and 0x00..0x0F to identify
 *           a key thas is pressed
 *
 * Columns - X1..X4 configured for Input;
 * Rows    - Y1..Y4 configured for Output;
 * Pull rows low;
 * Read columns;
 * If (no column low)
 *    return -1;
 * Assign column value;
 */
int keyPress(){

   unsigned long port; // IOPort value
   int column;
   int row;
   
   PullRowsLow();
   
/* Read IO port */
   port = __PIO_PDSR;
   
/* Return -1 if no key pressed */
   if ( ( port & X1 ) && ( port & X2 ) && ( port & X3 ) && ( port & X4 ) )
      return -1;
      
/* Calculate column position */
   column= calculateColumn( port );
   
/* Delay for key bounce interval */
   Delay_ms( 20 );

/* Read port again */
   port = __PIO_PDSR;   /* Pin Data Status Register */
   
/* Test for key bounce */
   if ( column != calculateColumn( port ) )
      return -1;
   
   if ( !(port & X1) )
      row= calculateRow( X1 );
   else if ( !(port & X2) )
      row= calculateRow( X2 );
   else if ( !(port & X3) )
      row= calculateRow( X3 );
   else if ( !(port & X4) )
      row= calculateRow( X4 );     
   else
      return -1;     

   return keyCode[row-1][column-1];
 
}
