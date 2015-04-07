
/*
 * $Revision: 1.2 $
 */

void UartInit(int(*getchar_func)(), void(*putchar_func)(int));
void UartRxrdy();

int ReceiveLine(char* line, int timeout);
void SendLine(char* line);

int RecvData( char* pData, int Size );
void SendData( char* pData, int Size );
