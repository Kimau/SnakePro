
/*
 * $Revision: 1.3 $
 */

void AT91_EB42_PllStart();
void AT91_EB55_PllStart();
void AT91EnablePeripheralClocks();
void AT91InitInterrupt(void(*timer_func)(), void(*rxrdy_func)());
void AT91InitTimer();
void AT91StartTimer();
void AT91UartInit();
int AT91UartGetchar();
void AT91UartPutchar(int ch);
unsigned int AT91GetButtons();

void AT91InitPIO();
void AT91LedSet(unsigned int mask);

