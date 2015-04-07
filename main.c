/*
 * XBeeTest
 *
 * Tests XBee module on AT91 board
 * Print any messages received and transmits if a key is depressed.
 *
 */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "timer.h"
#include "uart.h"
#include "AT91PIO.h"
#include "pg12864.h"
#include "keypad.h"
#include "Delay.h"
#include "sound.h"
#include "XBee.h"
#include "GameHeader.h"

void main(void) {
   int        key;        /* keycode */
   int        XBeeCode;   /* XBee initialisation code */
   SnakeMove  snakeBuffer;

/* Initialise GPIO */
   AT91InitialisePIO();
   
/* Initialize UART module and register getchar/putchar callbacks. */
   UartInit(AT91UartGetchar, AT91UartPutchar);
	
/* First disable interrupts. */
   __disable_interrupt();
	
/* Setup interrupt controller - for timer and UART Rx services */
   AT91InitInterrupt(TimerBeat, UartRxrdy);
	
/* Periodic timer initialization. */
   AT91InitTimer();
	
/* Setup serial port - baud rate, etc. */
   AT91UartInit();
	
/* Enable interrupts - timer and UART Rx services */
   __enable_interrupt();

/* Start periodic timer. */
   AT91StartTimer();

/* Initialise sound interface */
   soundInit();
   
/* Initialise LCD and clear display */
   LCD_Init();
   Delay_ms(500);
   LCD_ClearDisplay();

/* Title string */
   LCD_PutString("Starting Snake \n");
      
/* Initialise XBee module
   Channel= 0x0C
   PAN ID= 0x3330
   Source address= 0x0001
   Destination address= 0x0000 0000 0000 FFFF - a broadcast address */
   XBeeCode= XBeeInit( 0x0C, 0x3330, 0x0001, 0x00000000, 0x0000FFFF );
   
   if ( XBeeCode == 0 )
      LCD_PutString("XBee OK\n");
   else if ( XBeeCode == -2 )
      LCD_PutString("XBee init error\n");
   else if ( XBeeCode == -1 )
      LCD_PutString("XBee param error\n");
   else
     LCD_PutString("XBee FAILED! \n");

   s_GameInstance.m_updateCount = 0;
   s_GameInstance.m_currState = STATE_WAITING_FOR_HOST;
   
/* Main loop - print anything received and send message if key is pressed */
   while (TRUE) 
   {
     switch(s_GameInstance.m_currState)
     {
     case STATE_WAITING_FOR_HOST:
       {
         // Waiting Indicator
         LCD_PositionCursor(0,40);
         switch(s_GameInstance.m_updateCount % 10)
         {
         case  0:  LCD_PutString("-^-_______");   break;
         case  1:  LCD_PutString("_-^-______");   break;
         case  2:  LCD_PutString("__-^-_____");   break;
         case  3:  LCD_PutString("___-^-____");   break;
         case  4:  LCD_PutString("____-^-___");   break;
         case  5:  LCD_PutString("_____-^-__");   break;
         case  6:  LCD_PutString("______-^-_");   break;
         case  7:  LCD_PutString("_______-^-");   break;
         case  8:  LCD_PutString("-_______-^");   break;
         case  9:  LCD_PutString("^-_______-");   break;
         }        
         
         // Press a button to claim hostship
         if ( ( key= keyPress() ) == 0x0F ) 
         {
           LCD_ClearDisplay();
           LCD_PutString("Game Started \n");           
           StartGame(key);           
         }     
         // Check if someone is hosting Game
         else if( RecvData((char*)&snakeBuffer, sizeof(SnakeMove)) > 0)
         {
           // Check its a game starting and not a game in progress
           if(snakeBuffer.m_updateCount == 1)
           {
             LCD_ClearDisplay();
             LCD_PutString("Game Joined \n");
             JoinGame(&snakeBuffer);
           }
         }
         
         // DO NOT CLEAR SCREEN!!!
         Sleep(500);
       }
       break;
       
     case STATE_PLAYING:
       {
         // Normal Game loop
         UpdateNetwork();
         
         if(s_GameInstance.m_currState == STATE_PLAYING)
         {
           UpdateGame();
           UpdateScreen();
           UpdateInput();
         }         
       }
       break;
       
     case STATE_GAME_OVER:
       {
         // Display Game Over Message

         // Waiting 
         memset(&s_GameInstance, 0, sizeof(SnakeGame));
         s_GameInstance.m_currState = STATE_WAITING_FOR_HOST;         
       }
       break;
     }
     
     s_GameInstance.m_updateCount += 1;
   }        

}

