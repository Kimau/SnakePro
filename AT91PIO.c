/*
 * AT91PIO.c
 */

#include "config.h"
#include "AT91PIO.h"

/*
 * Clear a port bit
 */
void OutputLow( unsigned long bit ){
   __PIO_CODR = bit;
}


/*
 * Set a port bit
 */
void OutputHigh( unsigned long bit ){
   __PIO_SODR = bit;
}


/* No inputs in addition to keyboard column lines */
#define INPUTS  0x00000000

/* Outputs for LCD signals (P3, P4, P16, P17, P18, P19) and sounder (P6) */
#define OUTPUTS 0x000F0058

/* Four Outputs for row lines - P20, P21, P8, P23 */
#define ROWS    0x00B00100

/* Four inputs for column lines - P12, P9, P1, P2 */
#define COLUMNS 0x00001206

/* AT91InitialisePIO
 *
 * Initialise PIO register set for Powertip PG12864 LCD IO, Keyboard and Sounder
 *
 * I: Input
 * O: Output
 * x: Don't care
 *
 *  x x x x  x x x x  O x O O  O O O O  x x x I  x x I O  x O x O  O I I x
 * 31       27       23       19       15       11        7        3     0
 */
void AT91InitialisePIO() {
  __PIO_PER =  INPUTS | OUTPUTS | ROWS | COLUMNS;  // Enable PIO Controller for I/O bits
  __PIO_OER =  OUTPUTS | ROWS;                     // Output enable
  __PIO_ODR =  INPUTS | COLUMNS;                   // Output disable
  __PIO_CODR = OUTPUTS;                            // Clear Outputs
  __PIO_IDR =  INPUTS | ROWS | COLUMNS;            // Disable interrupts on Inputs
}



int getPortBit( unsigned long mask ) {
   unsigned long port;
   port = __PIO_PDSR;
   return ( port & mask );
 }

/* AT91GetInputBits
 *
 * Read all GPIO inputs
 *
 * WDH, June 22, 2006
 */
unsigned int AT91GetInputBits() {
  unsigned long pa; // Port A value.
  pa = ~__PIO_PDSR;   /* Pin Data Status Register */
  return (pa);
}


/* unsigned int GetButton( unsigned int button );
 *
 * Returns the state of a button
 */
unsigned int GetButton( unsigned int button ) {
    unsigned int buttonMask;
    buttonMask= AT91GetInputBits();
    if ( button & buttonMask )
      return 1;
    else
      return 0;
}
