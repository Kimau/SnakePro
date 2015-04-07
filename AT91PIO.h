/*
 * AT91PIO.h
 */

void OutputLow( unsigned long bit );
void OutputHigh( unsigned long bit );
void AT91InitialisePIO();
unsigned int AT91GetInputBits();
unsigned int GetButton( unsigned int );
int getPortBit( unsigned long mask );
