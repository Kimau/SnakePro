
/*
 * $Revision: DK October 2004 $
 */

#include "config.h"

static void(*timer_function)();
static void(*rxrdy_function)();


//
// Clock initialization.
//

void AT91EnablePeripheralClocks()
{
  // Switch on clocks to all peripherals.
  __PS_PCER = 0x00017c;
}


//
// Interrupt handlers.
//

 /* Timer interrupt handler */
__irq __arm void heartbeat_irq(void)
{
  // Called at 1000 Hz rate.
  __AIC_IVR = 0; // Debug variant of vector read, protected mode is used.
  
  (*timer_function)(); // Call timer callback function.
    
  __TC_SR; // Read timer/counter 0 status register.
  __AIC_EOICR = 0; // Signal end of interrupt to AIC.
}


/* Serial port RX interrupt handler */
__irq __arm void usart0_rxrdy_interrupt(void)
{
  __AIC_IVR = 0; // Debug variant of vector read, protected mode is used.

  (*rxrdy_function)(); // Call RX callback function.
      
  __AIC_EOICR = 0; // Signal end of interrupt to AIC.
}


/* Undefined interrupt handler */
__irq __arm void undefined_irq(void)
{
  __AIC_IVR = 0; // Debug variant of vector read, protected mode is used.

  // Do nothing.

  __AIC_EOICR = 0; // Signal end of interrupt to AIC.
}


//
// Interrupt functions.
//

void AT91InitInterrupt(void(*timer_func)(), void(*rxrdy_func)())
{
  int      irq_id ;

  timer_function = timer_func;
  rxrdy_function = rxrdy_func;
  
  // Disable all interrupts.
  __AIC_IDCR = 0xFFFFFFFF;
  // Clear all interrupts.
  __AIC_ICCR = 0xFFFFFFFF;

  // For each priority level.
  for (irq_id = 0; irq_id < 8; irq_id++)
  {
    // Unstack a level by writting in AIC_EOICR.
    // Value written has no effect.
    __AIC_EOICR = 0;
  }

  {
    // For each interrupt source.
    __REG32 volatile* aic_smr_base = &__AIC_SMR0;
    __REG32 volatile* aic_svr_base = &__AIC_SVR0;
    for (irq_id = 0; irq_id < 32; irq_id++)
    {
      // Priority is lowest.
      aic_smr_base[irq_id] = 0 ;
      // Interrupt routine is undefined.
      aic_svr_base[irq_id] = (unsigned long)&undefined_irq;
    }
  }
  
  __AIC_SPU = (unsigned long)&undefined_irq; // Spurious interrupt vector.
  __SF_PMR = 0x27a80020; // Run AIC in protected mode.
  // Initialize ARM IRQ vector to map to interrupt controller.
  *(unsigned long *)0x18 = 0xe51fff20; // ldr pc,[pc,#-0xf20]
}


//
// Timer functions.
//

void AT91InitTimer()
{
  __TC_IDR = 0xff; // Disable all timer/counter 0 interrupts.
  __AIC_SVR4 = (unsigned long)&heartbeat_irq;
  __AIC_SMR4 = 0x26;
  __AIC_ICCR_bit.tc0irq = 1; // Clears timer/counter 0 interrupt.
  __AIC_IECR_bit.tc0irq = 1; // Enable timer/counter 0 interrupt.
  
  __TC_CMR = 0x00004002; // Capture mode, CPCTRG=1, TCCLKS=2 (/32).
  __TC_RC = AT91_MCK / 32 / 4000; // Set RC (compare register), 0.25 ms interval.
  __TC_CCR = 1; // Enable the clock.
  __TC_CCR = 5; // Software trigger.
  __TC_CCR = 1; // Clear trigger.
}

void AT91StartTimer()
{
  __AIC_ICCR = 0x10; // Clears timer/counter 0 interrupt.
  __TC_SR; // Read timer/counter 0 status register to clear flags.
  __TC_IER_bit.cpcs = 1; // Interrupt on RC compare.
}


//
// Serial communication functions.
//

void AT91UartInit()
{
  __PIO_PDR = 0x000c000; // Disable PIO control of P14/TXD and P15/RXD.
  __US_MR = 0x000008c0; // Normal mode, 1 stop bit, no parity, async mode, 8 bits, MCK.
  __US_IDR = 0xffffffff; // Disable all USART interrupts.
  __US_IER = 1; // Interrupt on RXRDY.
  __US_TTGR = 5; // Transmit time guard in number of bit periods.
  __US_BRGR = AT91_MCK / BAUD_RATE / 16; // Set baud rate.

  __AIC_SVR2 = (unsigned long)&usart0_rxrdy_interrupt; // Usart 0 interrupt vector.
  __AIC_SMR2 = 0x63; // SRCTYPE=1, PRIOR=3. USART 0 interrupt positive edge-triggered at prio 3.
  __AIC_ICCR_bit.us0irq = 1; // Clears timer/counter 0 interrupt.
  __AIC_IECR_bit.us0irq = 1; // Enable timer/counter 0 interrupt.
  
  __AIC_ICCR = 1 << US0IRQ; // Clears usart 0 interrupt.

  __US_CR = 0x000000a0; // Disable receiver, disable transmitter.
  __US_CR = 0x0000010c; // Reset status bits, reset rx/tx.
  __US_CR = 0x00000050; // Enable receiver, enable transmitter.
}

  
int AT91UartGetchar()
{
  return __US_RHR;
}


void AT91UartPutchar(int ch)
{
  unsigned char status;

  do {
    status = __US_CSR;
  } while ((status & 0x02) == 0); // Wait for TXRDY

  __US_THR = ch;
}


//
// Parallel I/O functions.
//

void AT91InitPIO()
{
  __PIO_PER = 0xf527e; // enable register
  __PIO_PDR = 0x08000; // disable register
  __PIO_OER = 0xf4078; // output enable
  __PIO_ODR = 0x01206; // output disable
  __PIO_SODR = 0xf4078; // LED's off
  __PIO_IDR = 0x1206;
}
