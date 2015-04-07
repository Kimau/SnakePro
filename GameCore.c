#include "GameHeader.h"
#include <stdlib.h>
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

// Game Instance Varible
SnakeGame 	s_GameInstance;
char		s_bRedraw;

// ------ Functions

//----------------------------------------------------------------
// Assert
void PrintErrorMessage(char* errMsg, int a, int b, int c)
{
  char buffer[200];
  sprintf(buffer, "\n %i %i %i \n ------", a, b, c);
  
//  LCD_ClearDisplay();
  LCD_Home();
  LCD_PutString(errMsg);  
  LCD_PutString(buffer);
  s_bRedraw = TRUE;
  
  while( keyPress() == -1 ) 
  {
    // Do nothing
  }
  
}

//----------------------------------------------------------------
// Get Random Number
int RandomNumber()
{
  int r = (((s_GameInstance.m_randHold = s_GameInstance.m_randHold * 214013L + 2531011L) >> 16) & 0x7fff); 
  return r;
}

//----------------------------------------------------------------
// Redraw Score
void RedrawScore()
{
  char buffer[100];
  
  LCD_PositionCursor(1,0);
  sprintf(buffer, "P1:%03i  P2:%03i",  
          s_GameInstance.m_snakes[0].m_score, 
          s_GameInstance.m_snakes[1].m_score);
  LCD_PutString(buffer);
}

//----------------------------------------------------------------
// Draw Pick-up
void DrawPickup()
{
 // Draw Pick-up
  switch(s_GameInstance.m_pickupValue)
  {
    // ------------------------------------------
    // Small Pickup
    // #
  default:
  case 0:
    LCD_SetPixel(s_GameInstance.m_pickupPos[X], s_GameInstance.m_pickupPos[Y]);
    break;

    // ------------------------------------------
    // Pickup
    //  #
    // # #
    //  #
  case 1:
    LCD_SetPixel(s_GameInstance.m_pickupPos[X], s_GameInstance.m_pickupPos[Y]+1);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X], s_GameInstance.m_pickupPos[Y]-1);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]+1, s_GameInstance.m_pickupPos[Y]);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]-1, s_GameInstance.m_pickupPos[Y]);
    break;
  
    // ------------------------------------------
    // Big Pickup
    // ###
    // # #
    // ###
  case 2:
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]-1, s_GameInstance.m_pickupPos[Y]-1);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]-1, s_GameInstance.m_pickupPos[Y]);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]-1, s_GameInstance.m_pickupPos[Y]+1);

    LCD_SetPixel(s_GameInstance.m_pickupPos[X], s_GameInstance.m_pickupPos[Y]);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X], s_GameInstance.m_pickupPos[Y]);

    LCD_SetPixel(s_GameInstance.m_pickupPos[X]+1, s_GameInstance.m_pickupPos[Y]-1);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]+1, s_GameInstance.m_pickupPos[Y]);
    LCD_SetPixel(s_GameInstance.m_pickupPos[X]+1, s_GameInstance.m_pickupPos[Y]+1);
    break;
  }
}

//----------------------------------------------------------------
// Redraw full game Screen
void RedrawFullGame()
{
  int xPos = 1;
  int yPos = 0;
  
  // Clear Display
  LCD_ClearDisplay();

  // Render Score
  RedrawScore();

  // Draw Box
  xPos = 0;
  while(xPos < (LCD_X_MAX*9))
  {
    LCD_SetPixel(xPos, PLAY_OFFSETY-1);
    LCD_SetPixel(xPos, PLAY_OFFSETY+PLAY_HEIGHT+1);
    xPos += 1;
  }
  
  yPos = 8;
  while(yPos < (LCD_Y_MAX*9))
  {
    LCD_SetPixel( PLAY_OFFSETX-1, yPos );
    LCD_SetPixel( PLAY_OFFSETX+PLAY_WIDTH+1, yPos );
    yPos += 1;
  }

  // Draw Full Snake
  for(int iPlay = 0; iPlay < 2; ++iPlay)
  {
    // Draw head
    LCD_SetPixel(s_GameInstance.m_snakes[iPlay].m_head[X], s_GameInstance.m_snakes[iPlay].m_head[Y]);

    // Local Player is always solid
    if(iPlay == !s_GameInstance.m_isHost) 
    {
      // Draw Tail
      for(int iLength = 0; iLength < s_GameInstance.m_snakes[iPlay].m_length; ++iLength)      
      {
        if((s_GameInstance.m_snakes[iPlay].m_tailP[iLength][X] + s_GameInstance.m_snakes[iPlay].m_tailP[iLength][Y]) > 0)
        {
          LCD_SetPixel(s_GameInstance.m_snakes[iPlay].m_tailP[iLength][X], 
                       s_GameInstance.m_snakes[iPlay].m_tailP[iLength][Y]);
        }
        else
        {
          return;
        }
      }
    }
    // Player 2 is dashed
    else
    {
      // Draw Tail
      for(int iLength = 0; iLength < s_GameInstance.m_snakes[iPlay].m_length; ++iLength)      
      {
        // Tail Piece is always drawn
        if ( (iLength == (s_GameInstance.m_snakes[iPlay].m_length - 1)) || 
             ((s_GameInstance.m_snakes[iPlay].m_tailP[iLength+1][X] + s_GameInstance.m_snakes[iPlay].m_tailP[iLength+1][Y]) == 0))
        {
          LCD_SetPixel(s_GameInstance.m_snakes[iPlay].m_tailP[iLength][X], 
                       s_GameInstance.m_snakes[iPlay].m_tailP[iLength][Y]);
          return;
        }

        // Draw Piece
        if((iLength % 3) != 0)
        {
          LCD_SetPixel(s_GameInstance.m_snakes[iPlay].m_tailP[iLength][X], 
                       s_GameInstance.m_snakes[iPlay].m_tailP[iLength][Y]);
        }
      }
    }
  }
  
  DrawPickup();
}

//----------------------------------------------------------------
// Setup Snake Game Parameters
void SetupGame()
{
  s_GameInstance.m_randHold = s_GameInstance.m_randSeed;
  
  // Setup Game
  s_GameInstance.m_pickupPos[0] = 0;
  s_GameInstance.m_pickupPos[1] = 0;
  s_GameInstance.m_pickupValue = 0;
    
  memset(&(s_GameInstance.m_snakes[0]), NULL, sizeof(SnakeData));
  memset(&(s_GameInstance.m_snakes[1]), NULL, sizeof(SnakeData));
  
  s_GameInstance.m_snakes[0].m_head[X] = 16;
  s_GameInstance.m_snakes[0].m_head[Y] = 28;
  s_GameInstance.m_snakes[0].m_length = 4;
  s_GameInstance.m_snakes[0].m_dir = EAST;
  
  s_GameInstance.m_snakes[1].m_head[X] = 110;
  s_GameInstance.m_snakes[1].m_head[Y] = 28;
  s_GameInstance.m_snakes[1].m_length = 4;
  s_GameInstance.m_snakes[1].m_dir = WEST;
  
  // Generate first pickup
  GeneratePickup();  
  
  // Setup Screen
  s_bRedraw = TRUE;
  
  // Update State
  s_GameInstance.m_currState = STATE_PLAYING;
}


//----------------------------------------------------------------
// Start Hosting a Game
void StartGame(int StartKey)
{
  // Sound Feedback
  // Click();
  
  // Setup Random Seed based on running time and key
  s_GameInstance.m_randSeed = s_GameInstance.m_updateCount + StartKey;
  s_GameInstance.m_randHold = s_GameInstance.m_randSeed;
  s_GameInstance.m_updateCount = 0;
    
  // Claim Host Status 
  s_GameInstance.m_isHost = TRUE;
  s_GameInstance.m_prevClientMove = NO_MOVE;
  
  // Setup Game of Snake
  SetupGame();
}

//----------------------------------------------------------------
// Join Running Game
void JoinGame(SnakeMove* pRecieveMove)
{
  // Sound Feedback
  // Click();
  
  // Setup Random Seed based on network Data
  s_GameInstance.m_updateCount      = 0;
  s_GameInstance.m_randSeed         = pRecieveMove->m_randHold;
  s_GameInstance.m_randHold         = pRecieveMove->m_randHold;
  
  // Claim Host Status 
  s_GameInstance.m_isHost = FALSE;
  s_GameInstance.m_prevClientMove = NO_MOVE;
  
  // Setup Game of Snake
  SetupGame();
}

//----------------------------------------------------------------
// Transmit Local Move Data
void TransmitLocalMove(char snakeDir, int turnOffset)
{
  // Setup Move
  SnakeMove moveData;
  moveData.m_currDir      = snakeDir;
  moveData.m_updateCount  = s_GameInstance.m_updateCount + turnOffset;
  moveData.m_randHold     = s_GameInstance.m_randSeed;

  // Send Move  
  SendData( (char*)&moveData, sizeof(SnakeMove) ); 
}

//----------------------------------------------------------------
// Process and Sanitize Network Reciecved Input
char ProcessRecievedMove(SnakeMove* pRecvMove)
{
  // Safety Checks
  if(pRecvMove == NULL)
  {
    return FALSE;
  }
  
  // Update Frame Check
  if(pRecvMove->m_updateCount != s_GameInstance.m_updateCount)
  {
    // PrintErrorMessage("Update Frame Mismatch", pRecvMove->m_updateCount, s_GameInstance.m_updateCount, NULL);
    return FALSE;
  }
  
  // Compare Rand Hold
  if(pRecvMove->m_randHold != s_GameInstance.m_randSeed)
  {
    // PrintErrorMessage("Random Number Mismatch", pRecvMove->m_randHold, s_GameInstance.m_randHold, NULL);
    return FALSE;
  }
  
  // Update
  if(pRecvMove->m_currDir != NO_MOVE)
  {
    s_GameInstance.m_snakes[s_GameInstance.m_isHost].m_dir = pRecvMove->m_currDir;
  }
  
  return TRUE;
}

//----------------------------------------------------------------
// LockStep Send
void LockStep(char snakeDir, int frameOffset)
{
  SnakeMove moveBuffer;
  
  while(TRUE)
  {
    memset(&moveBuffer, 0, sizeof(SnakeMove));
    
    // Listen for Response or till time out
    if( RecvData((char*)&moveBuffer, sizeof(SnakeMove)) > 0)
    {
      // Move Recieved: Advance
      if(ProcessRecievedMove(&moveBuffer) == TRUE)
      {
        return;
      }
      
    }
    
    // Timed-out: Resend Host Move
    TransmitLocalMove(snakeDir, frameOffset);
    
    // Cancel Lock
    if(keyPress() == 0x0C)
    {
      s_GameInstance.m_currState = STATE_WAITING_FOR_HOST; 
      return;
    }
    
    Sleep(100);
  }
}

//----------------------------------------------------------------
// Update Network Host
void UpdateNetHost()
{
  // Send Host Move
  TransmitLocalMove(s_GameInstance.m_snakes[0].m_dir, 0);
 
  LockStep(s_GameInstance.m_snakes[0].m_dir, 0);
}

//----------------------------------------------------------------
// Update Network on Client
void UpdateNetClient()
{
  LockStep(s_GameInstance.m_prevClientMove, -1);
    
  // Send Client Move
  TransmitLocalMove(s_GameInstance.m_snakes[1].m_dir, 0);
}

//----------------------------------------------------------------
// Update Network
void UpdateNetwork()
{
  if(s_GameInstance.m_isHost == TRUE)
  {
    UpdateNetHost();
  }
  else
  {
    UpdateNetClient();
  }
}

//----------------------------------------------------------------
// Update Pos
void UpdatePos(unsigned char* pPos, char dir)
{
  if(pPos == NULL)
  {
    return;
  }
  
  switch(dir)
  {
    case NORTH: pPos[Y]--;     break;
    case EAST:  pPos[X]++;     break;
    case SOUTH: pPos[Y]++;     break;
    case WEST:  pPos[X]--;     break;
  }
}

//----------------------------------------------------------------
// End the Game and Set Winner (-1 == Draw)
void EndGame(int winner)
{
  // Safety Check
  RedrawFullGame();

  // Draw Text on Top
  LCD_Home();

  if(winner < 0)
  {
    LCD_PutString("GAME DRAWN");
  }
  else if(winner != s_GameInstance.m_isHost)
  {
    LCD_PutString("WINNER...");
  }
  else
  {
    LCD_PutString("LOSER....");
  }

  // Play Little Fanfare
  {
    // E F G C DEF GABF ABCDE EFGC DEF GGED GED GED GFEDC
    playNote(e3, 4);
    Sleep(50);
    
    playNote(f3, 4);
    Sleep(50);
    
    playNote(g3, 4);
    Sleep(50);
    
    playNote(c3, 4);
    Sleep(50);
    
    playNote(d3, 1);
    playNote(e3, 1);
    playNote(f3, 2);
    Sleep(50);
    
    playNote(g3, 1);
    playNote(a3, 1);
    playNote(b3, 1);
    playNote(f3, 1);
    Sleep(50);
    
    playNote(a3, 1);
    playNote(b3, 1);
    playNote(c3, 1);
    playNote(d3, 1);
    playNote(e3, 1);
    Sleep(50);
    
    playNote(e3, 1);
    playNote(f3, 1);
    playNote(g3, 1);
    playNote(c3, 1);
    Sleep(50);
    
    playNote(d3, 1);
    playNote(e3, 1);
    playNote(f3, 2);
    Sleep(50);
    
    playNote(g3, 1);
    playNote(g3, 1);
    playNote(e3, 1);
    playNote(d3, 1);
    Sleep(50);
    
    playNote(g3, 1);
    playNote(e3, 1);
    playNote(d3, 2);
    Sleep(50);
    
    playNote(g3, 1);
    playNote(e3, 1);
    playNote(d3, 2);
    Sleep(50);
    
    playNote(g3, 1);
    playNote(e3, 1);
    playNote(f3, 1);
    playNote(d3, 1);
    playNote(c3, 1);
  }
  
  Sleep(3000);
  
  LCD_ClearDisplay();
  LCD_Home();
  LCD_PutString("Game Lobby \n");

  // Update State
  s_GameInstance.m_currState = STATE_GAME_OVER;
}

//----------------------------------------------------------------
// Compare Two Positions
char ComparePositions(const unsigned char* posA, const unsigned char* posB)
{
  if((posA[0] == posB[0]) && (posA[1] == posB[1]))
  {
    return TRUE;
  }
  
  return FALSE;
}

//----------------------------------------------------------------
// Collision Sweep against Snakes
char CollisionSweep(unsigned char* testPos)
{
  for(int iPlay = 0; iPlay < 2; ++iPlay)
  {
    // Check head
    if(ComparePositions(testPos, s_GameInstance.m_snakes[iPlay].m_head) == TRUE)
    {
      // Collision and DEATH
      iPlay = 3;    // Little trick to break sooner
      return TRUE;        
    }
    
    // Check Borders
    if((testPos[X] < PLAY_OFFSETX) || 
       (testPos[Y] < PLAY_OFFSETY) || 
       (testPos[X] > (PLAY_OFFSETX + PLAY_WIDTH)) || 
       (testPos[Y] > (PLAY_OFFSETY + PLAY_HEIGHT)) )
    {
        // Outside Bounds
        iPlay = 3;    // Little trick to break sooner
        return TRUE;
    }
  
    // Check Tail
    for(int iLength = 0; iLength < s_GameInstance.m_snakes[iPlay].m_length; ++iLength)      
    {
      if((s_GameInstance.m_snakes[iPlay].m_tailP[iLength][0] + s_GameInstance.m_snakes[iPlay].m_tailP[iLength][1]) == 0)
      {
        // Invalid Position ignore
      }
      else
      {
        if(ComparePositions(testPos, s_GameInstance.m_snakes[iPlay].m_tailP[iLength]) == TRUE)
        {
          // Collision and DEATH
          iPlay = 3;    // Little trick to break sooner
          return TRUE;
        }
      }
    }
  }
  
  return FALSE;
}

//----------------------------------------------------------------
// Update Snake
void UpdateSnake(int iPlay, SnakeData* pSnakeData)
{
  // Get Future Position
  unsigned char newPos[2] = 
  {
    pSnakeData->m_head[X], 
    pSnakeData->m_head[Y]
  };
  
  UpdatePos(newPos, pSnakeData->m_dir);
  
  // Against Pickup
  if(ComparePositions(newPos, s_GameInstance.m_pickupPos) == TRUE)
  {
    pSnakeData->m_score += s_GameInstance.m_pickupValue * 5;
    pSnakeData->m_length += (s_GameInstance.m_pickupValue + 1) * 2;

    // Play Pickup Noise
    playNote(a4, 2);
    playNote(b4, 2);

    // Make New Pickup
    GeneratePickup(); 

    // Redraw full screen to avoid artifacts from pick ups
    s_bRedraw = TRUE;
  }
  // Against Snake 
  else if(CollisionSweep(newPos) == TRUE)
  {
    EndGame(!iPlay);   
  }
  
  // Move Head Forward
  int iTail = pSnakeData->m_length;
  while(iTail > 0)
  {
    pSnakeData->m_tailP[iTail][X] = pSnakeData->m_tailP[iTail-1][X];
    pSnakeData->m_tailP[iTail][Y] = pSnakeData->m_tailP[iTail-1][Y];
    
    --iTail;
  }
  
  // Push Head onto Tail
  pSnakeData->m_tailP[0][X] = pSnakeData->m_head[X];
  pSnakeData->m_tailP[0][Y] = pSnakeData->m_head[Y];
  
  // Update Head
  pSnakeData->m_head[X] = newPos[X];
  pSnakeData->m_head[Y] = newPos[Y];
}

//----------------------------------------------------------------
// Update Game
void UpdateGame()
{
  // Special Case : Both Snakes going to same square
  unsigned char newPos[2][2] = 
  {
    { 
      s_GameInstance.m_snakes[0].m_head[X], 
      s_GameInstance.m_snakes[0].m_head[Y] 
    },
    { 
      s_GameInstance.m_snakes[1].m_head[X], 
      s_GameInstance.m_snakes[1].m_head[Y] 
    },
  };
  
  // Update Position
  UpdatePos(newPos[0], s_GameInstance.m_snakes[0].m_dir);
  UpdatePos(newPos[1], s_GameInstance.m_snakes[1].m_dir);
  
  if(ComparePositions(newPos[0], newPos[1]))
  {
    // Both Moving into Same Space
    EndGame(-1);
  }
  
  // Update Snake
  UpdateSnake(0, &(s_GameInstance.m_snakes[0]));
  UpdateSnake(1, &(s_GameInstance.m_snakes[1]));

  // Update Pickup Timer
  if(s_GameInstance.m_pickupTime > 1)
  {
    s_GameInstance.m_pickupTime -= 1;
  }
  else
  {
    GeneratePickup();
  }
  
}

//----------------------------------------------------------------
// Generate Pickup
//
void GeneratePickup()
{
  playNote(c4, 2);
  playNote(e4, 2);

  // Get New Value
  s_GameInstance.m_pickupValue = (s_GameInstance.m_pickupValue + 1) % 3;
  s_GameInstance.m_pickupTime = (s_GameInstance.m_pickupValue + 1) * 100;
    
  int passTest = FALSE;  
  while(passTest == FALSE)
  {
    // Pass Test
    passTest = TRUE;
    
    // Pick a New Position
    s_GameInstance.m_pickupPos[X] = (RandomNumber() % PLAY_WIDTH)  + PLAY_OFFSETX;
    s_GameInstance.m_pickupPos[Y] = (RandomNumber() % PLAY_HEIGHT) + PLAY_OFFSETY;
    
    // Collision Check
    if(CollisionSweep(s_GameInstance.m_pickupPos) == TRUE)
    {
      // Fail so retry
      passTest = FALSE;
    }
  }
  
  // Done
}

//----------------------------------------------------------------
// PauseGame
void PauseGame() 
{
  int key = keyPress();
  
  // Infinite Loop effectively pauses the game
  // Then because the networking is lock-stepped there is no advancement on the client
  while(TRUE)
  {
    key = keyPress();
    
    if( key == 0x05 )
    {
      return;
    }
  }
}

//----------------------------------------------------------------
// Update Input
// This just needs to correct the direction of travel
void UpdateInput()
{
  int key = keyPress();
  
  if( key != -1 )
  {
    switch (key)
    {
    case 0x02:	s_GameInstance.m_snakes[!s_GameInstance.m_isHost].m_dir = NORTH;    break;
    case 0x04:	s_GameInstance.m_snakes[!s_GameInstance.m_isHost].m_dir = WEST;     break;
    case 0x06:	s_GameInstance.m_snakes[!s_GameInstance.m_isHost].m_dir = EAST;     break;
    case 0x08:	s_GameInstance.m_snakes[!s_GameInstance.m_isHost].m_dir = SOUTH;    break;
  
    case 0x05: // Pause Game
      if(s_GameInstance.m_currState == STATE_PLAYING)
      {
        PauseGame();
      }
      break;
     
    case 0x0C:
      s_GameInstance.m_currState = STATE_WAITING_FOR_HOST;      
      break;
    }
  }
}

//----------------------------------------------------------------
// Minimal Render Update of Snake
void MinRenderSnake(int iPlay, SnakeData* pSnakeData)
{
  // Clear Tail
  if((pSnakeData->m_tailP[pSnakeData->m_length][X] + pSnakeData->m_tailP[pSnakeData->m_length][Y]) > 0)
  {
    LCD_ClearPixel(pSnakeData->m_tailP[pSnakeData->m_length][X], 
                   pSnakeData->m_tailP[pSnakeData->m_length][Y]);
  }
  
  // Draw Head
  LCD_SetPixel(pSnakeData->m_head[X], pSnakeData->m_head[Y]);

  if(iPlay == s_GameInstance.m_isHost) 
  {
    // Draw Tail (might have been blank)
    LCD_SetPixel(pSnakeData->m_tailP[pSnakeData->m_length-1][X], 
                 pSnakeData->m_tailP[pSnakeData->m_length-1][Y]);

    // Clear every 3rd pixel
    if((s_GameInstance.m_updateCount % 3) == 0)
    {
      LCD_ClearPixel(pSnakeData->m_tailP[0][X], pSnakeData->m_tailP[0][Y]);
    }
  }
}

//----------------------------------------------------------------
// Update Screen
void UpdateScreen()
{
  // Fullscreen Redraw
  if(s_bRedraw == TRUE)
  {
    RedrawFullGame();
    s_bRedraw = FALSE;
  }
  
  MinRenderSnake(0, &(s_GameInstance.m_snakes[0]));
  MinRenderSnake(1, &(s_GameInstance.m_snakes[1]));
  DrawPickup();
}
