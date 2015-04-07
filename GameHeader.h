//

#define MAX_SNAKE_LENGTH 100

// Directions are stored as

#define NO_MOVE 0
#define NORTH 	1
#define EAST 	2
#define SOUTH 	3
#define WEST 	4

#define TRUE  1
#define FALSE 0

#define X 0
#define Y 1

#define STATE_WAITING_FOR_HOST 	0
#define STATE_PLAYING 		1
#define STATE_GAME_OVER		2

#define PLAY_WIDTH   125
#define PLAY_HEIGHT  53
#define PLAY_OFFSETX 1
#define PLAY_OFFSETY 9

typedef struct SnakeData_
{
	unsigned char 	m_dir; 		 		// Snake current facing Position
	unsigned char 	m_length; 		 	// Current Length NOT including Tail
	unsigned char 	m_head[2]; 	 		// Snake head Position X:Y
	unsigned char   m_tailP[MAX_SNAKE_LENGTH+1][2];	// Absolute Tail pieces
	int 	        m_score; 			// Current Score
} SnakeData;

typedef struct SnakeGame_
{
	int             m_updateCount;      // Tracks Update Loop Count
	int 		m_randSeed;         // Initial Random Seed
	int 		m_randHold;         // Holding Last Random Iteration
 	unsigned char   m_isHost; 	    // Am I the Host
	unsigned char   m_pickupPos[2];     // Reward Position X:Y
	unsigned char   m_pickupValue;      // Pickup Value
        unsigned char   m_pickupTime;       // Pickup Timer
	unsigned char   m_currState;        // current State
	unsigned char   m_prevClientMove;   // Need to store for timeout situation
 	SnakeData	m_snakes[2];        // Snake Data
} SnakeGame;

typedef struct SnakeMove_
{
   unsigned char  m_currDir;          // My current direction of travel
   int            m_updateCount;      // Current Frame
   int            m_randHold;         // Last rand used
} SnakeMove;

extern SnakeGame s_GameInstance;

// ------ Functions
void StartGame(int StartKey);
void JoinGame(SnakeMove* pRecieveMove);
void UpdateNetwork();
void UpdateGame();
void UpdateInput();
void UpdateScreen();
void DrawGameOver();
void GeneratePickup();
char ProcessRecievedMove(SnakeMove* pRecvMove);



