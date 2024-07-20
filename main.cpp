#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <math.h>

bool run = true;

int wWidth = 800,wHeight = 680;
HDC hdcGui;
HINSTANCE hinsGui;
tagBITMAPINFO biGui;

int* visualbg = (int*)calloc(640*480,4);
int* visual = (int*)calloc(640*480,4);
int* visuala = (int*)calloc(640*480,4);

HBITMAP hbmLoadBmps;

char EngPath1[MAX_PATH] = "engine1.exe";
char EngPath2[MAX_PATH] = "engine2.exe";

PROCESS_INFORMATION proc1;
HANDLE eng1In;
HANDLE eng1Out;

PROCESS_INFORMATION proc2;
HANDLE eng2In;
HANDLE eng2Out;

char* textToEng = (char*)calloc(1000000,1);
int aTextLength = 0;

char* textFromEng = (char*)calloc(1000000,1);
int textFromPos = 0;
char* bmLoc;

int* chars = (int*)calloc(1024*15,4);

int AntiAl = 1;
int loseIfFlagged = 1;
int BotAutoNewGame = 1;
int AutoNGDelay = 1500;
int aDelayStarted = 0;
int sTime = 10000;
int vsPrPlBy = 2;
int prClickedRow = 0;
int prClickedCol = 0;
int prClicked = 0;

int* board1 = (int*)calloc(8*8,4);
int State = 0;
int prCol = 0;
int Thinking = 0;
char turn = 1;
int flipBrd = 0;
int LastMove = 0;
int MoveNum = 1;
int EngThinking = 0;
int needRestart = 0;
int drawnPrMove = 1;
char* MoveList = (char*)calloc(10000,1);
char* aMLToShow = (char*)calloc(32,1);
int MoveListPos = 0;
int lastX = 0;
int lastY = 0;
int lastStartX = 0;
int lastStartY = 0;
int ClockW = sTime;
int ClockB = sTime;
int MoveTimeStart = -1;
int castle = 0;
int isFlagged = 0;
char aClockText[16];
char StartPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 10";

int min(int a, int b)
{
	return a < b ? a : b;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

int baLen(char* bArray)
{
	int len = -1;
	while (bArray[++len] != '\0') ;
	return len;
}

void fillMem(int* des,int value,int count)
{
	int aPos = -1;
	while (++aPos < count) *(des + aPos) = value;
}

void drawRect(int* des,int dWidth,int dx,int dy,int delx,int dely,int color)
{
	int ax = 0,ay = 0;
	while (ay < dely)
	{
		*(des + (dy + ay) * dWidth + (dx + ax)) = color;
		
		ax += 1;
		if (ax >= delx)
		{
			ax = 0;
			ay += 1;
		}
	}
}

void copyRect(int* des,int* src,int sWidth,int dWidth,int sx,int sy,int dx,int dy,int delx,int dely,int bgColor)
{
	int ax = 0,ay = 0;
	while (ay < dely)
	{
		if (*(src + (sy + ay) * sWidth + (sx + ax)) != bgColor) *(des + (dy + ay) * dWidth + (dx + ax)) = *(src + (sy + ay) * sWidth + (sx + ax));
		
		ax += 1;
		if (ax >= delx)
		{
			ax = 0;
			ay += 1;
		}
	}
}

void stretchRect(int* des,int* src,int sWidth,int dWidth,int sx,int sy,int sDelX,int sDelY,int dx,int dy,int dDelX,int dDelY,int bgCol)
{
	int ax = 0,ay = 0;
	float xr = (float)sDelX / (float)dDelX , yr = (float)sDelY / (float)dDelY;
	
	while (ay < dDelY)
	{
		int sCol = *(src + (sy + lround((float)ay * yr)) * sWidth + (sx + lround((float)ax * xr)));
		if (sCol != bgCol) *(des + (dy + ay) * dWidth + (dx + ax)) = sCol;
		
		ax += 1;
		if (ax >= dDelX)
		{
			ax = 0;
			ay += 1;
		}
	}
}

void blendRect(int* des,int* src,int sWidth,int dWidth,int sx,int sy,int dx,int dy,int delx,int dely)
{
	int ax = 0,ay = 0;
	while (ay < dely)
	{
		int ax1 = max(ax - 1, sx);
		int ax2 = min(ax + 1, sx + delx - 1);
		int ay1 = max(ay - 1, sy);
		int ay2 = min(ay + 1, sy + dely - 1);
		
		int col1 = *(src + (sy + ay) * sWidth + (sx + ax));
		int col2 = *(src + (sy + ay1) * sWidth + (sx + ax));
		int col3 = *(src + (sy + ay2) * sWidth + (sx + ax));
		int col4 = *(src + (sy + ay) * sWidth + (sx + ax1));
		int col5 = *(src + (sy + ay) * sWidth + (sx + ax2));
		
		unsigned char* desPtr = (unsigned char*)(des + (dy + ay) * dWidth + (dx + ax));
		
		desPtr[0] = ((int)*((unsigned char*)&col1 + 0) * 7 + (int)*((unsigned char*)&col2 + 0) + (int)*((unsigned char*)&col3 + 0) + (int)*((unsigned char*)&col4 + 0) + (int)*((unsigned char*)&col5 + 0)) / 11;
		desPtr[1] = ((int)*((unsigned char*)&col1 + 1) * 7 + (int)*((unsigned char*)&col2 + 1) + (int)*((unsigned char*)&col3 + 1) + (int)*((unsigned char*)&col4 + 1) + (int)*((unsigned char*)&col5 + 1)) / 11;
		desPtr[2] = ((int)*((unsigned char*)&col1 + 2) * 7 + (int)*((unsigned char*)&col2 + 2) + (int)*((unsigned char*)&col3 + 2) + (int)*((unsigned char*)&col4 + 2) + (int)*((unsigned char*)&col5 + 2)) / 11;
		
		ax += 1;
		if (ax >= delx)
		{
			ax = 0;
			ay += 1;
		}
	}
}

void writeTextRect(int* chars,char* text,int* des,int width,int x,int y,int dx,int dy,int color)
{
	int len = baLen(text);

	int charWidth = 10;
	
	int aChr = -1;
	while (++aChr < len)
	{
		stretchRect(des,chars,1024,width,((unsigned char)text[aChr] < 127 ? text[aChr] : 63) * 8,0,8,14,x + aChr * charWidth,y,charWidth,dy,0xFFFFFF);
		
	}
}

void toFen(char* fen,int* board,int whosTurn,char castleInfo,int moveNum)
{
	ZeroMemory(fen,100);
	
	int fenPos = 0;
	int boardPos = 0;
	
	while (1)
	{
		if (board[boardPos] == 0) fen[fenPos] = '1';
		else if (board[boardPos] == 1) fen[fenPos] = 'P';
		else if (board[boardPos] == 2) fen[fenPos] = 'N';
		else if (board[boardPos] == 3) fen[fenPos] = 'B';
		else if (board[boardPos] == 4) fen[fenPos] = 'R';
		else if (board[boardPos] == 5) fen[fenPos] = 'Q';
		else if (board[boardPos] == 6) fen[fenPos] = 'K';
		else if (board[boardPos] == 7) fen[fenPos] = 'p';
		else if (board[boardPos] == 8) fen[fenPos] = 'n';
		else if (board[boardPos] == 9) fen[fenPos] = 'b';
		else if (board[boardPos] == 10) fen[fenPos] = 'r';
		else if (board[boardPos] == 11) fen[fenPos] = 'q';
		else if (board[boardPos] == 12) fen[fenPos] = 'k';
		
		fenPos += 1;
		
		boardPos += 1;
		if (boardPos > 63) break;
		
		if (boardPos % 8 == 0) 
		{
			fen[fenPos] = '/';
			fenPos += 1;
		}
	}
	
	if (whosTurn == 1) memcpy(fen + fenPos," w ",3);
	else if (whosTurn == 2) memcpy(fen + fenPos," b ",3);
	fenPos += 3;
	
	if ((castleInfo & 1) == 0) 
	{
		memcpy(fen + fenPos,"K",1);
		fenPos += 1;
	}
	if ((castleInfo & 2) == 0) 
	{
		memcpy(fen + fenPos,"Q",1);
		fenPos += 1;
	}
	if ((castleInfo & 4) == 0) 
	{
		memcpy(fen + fenPos,"k",1);
		fenPos += 1;
	}
	if ((castleInfo & 8) == 0) 
	{
		memcpy(fen + fenPos,"q",1);
		fenPos += 1;
	}
	if (castleInfo == (1|2|4|8))
	{
		memcpy(fen + fenPos,"-",1);
		fenPos += 1;
	}
	
	sprintf(fen + fenPos," - 5 %d",moveNum);
}

void fenToBrd(char* fen,int* board)
{
	int fenPos = -1;
	int boardPos = 0;
	
	while (fen[++fenPos] != ' ')
	{
		if (fen[fenPos] >= 48 && fen[fenPos] <= 57)
		{
			fillMem(board + boardPos,0,fen[fenPos] - 48);
			boardPos += fen[fenPos] - 48;
		}
		else 
		{
			if (fen[fenPos] == '/') continue;
			else if (fen[fenPos] == 'P') board[boardPos] = 1;
			else if (fen[fenPos] == 'N') board[boardPos] = 2;
			else if (fen[fenPos] == 'B') board[boardPos] = 3;
			else if (fen[fenPos] == 'R') board[boardPos] = 4;
			else if (fen[fenPos] == 'Q') board[boardPos] = 5;
			else if (fen[fenPos] == 'K') board[boardPos] = 6;
			else if (fen[fenPos] == 'p') board[boardPos] = 7;
			else if (fen[fenPos] == 'n') board[boardPos] = 8;
			else if (fen[fenPos] == 'b') board[boardPos] = 9;
			else if (fen[fenPos] == 'r') board[boardPos] = 10;
			else if (fen[fenPos] == 'q') board[boardPos] = 11;
			else if (fen[fenPos] == 'k') board[boardPos] = 12;
			else boardPos -= 1;
		
			boardPos += 1;
		}
	}
}

bool startProcess(HANDLE* pipeChildInWrite,HANDLE* pipeChildOutRead,char* eng,PROCESS_INFORMATION* engiProcInfo)
{
	HANDLE pipeChildInRead;
	HANDLE pipeChildOutWrite;
	
	SECURITY_ATTRIBUTES inheritableSa;
	inheritableSa.nLength = sizeof(SECURITY_ATTRIBUTES);
	inheritableSa.lpSecurityDescriptor = 0;
	inheritableSa.bInheritHandle = TRUE;
	
	CreatePipe(&pipeChildInRead,pipeChildInWrite,&inheritableSa,1000000);
	SetHandleInformation(*pipeChildInWrite,HANDLE_FLAG_INHERIT,0);
	CreatePipe(pipeChildOutRead,&pipeChildOutWrite,&inheritableSa,1000000);
	SetHandleInformation(*pipeChildOutRead,HANDLE_FLAG_INHERIT,0);
	
	STARTUPINFO engiStartupInfo;
	ZeroMemory(&engiStartupInfo,sizeof(STARTUPINFO));
	engiStartupInfo.cb = sizeof(STARTUPINFO);
	engiStartupInfo.dwFlags = STARTF_USESTDHANDLES;
	engiStartupInfo.hStdInput = pipeChildInRead;
	engiStartupInfo.hStdOutput = pipeChildOutWrite;
	
	ZeroMemory(engiProcInfo,sizeof(PROCESS_INFORMATION));
	
	return CreateProcess(eng,0,0,0,1,CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS,0,".",&engiStartupInfo,engiProcInfo);
}

bool playMove(int* board,char* move)
{
	board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = board[(56 - (int)move[1]) * 8 + ((int)move[0] - 97)];
	if (move[4] == ' ') ;
	else if (move[4] == 'n' && move[3] == '8') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 2;
	else if (move[4] == 'n' && move[3] == '1') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 8;
	else if (move[4] == 'b' && move[3] == '8') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 3;
	else if (move[4] == 'b' && move[3] == '1') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 9;
	else if (move[4] == 'r' && move[3] == '8') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 4;
	else if (move[4] == 'r' && move[3] == '1') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 10;
	else if (move[4] == 'q' && move[3] == '8') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 5;
	else if (move[4] == 'q' && move[3] == '1') board[(56 - (int)move[3]) * 8 + ((int)move[2] - 97)] = 11;
		
	board[(56 - (int)move[1]) * 8 + ((int)move[0] - 97)] = 0; 
	return true;
}

void setLastMove(char* move)
{
	lastX = (int)move[2] - 97;
	lastY = 56 - (int)move[3];
	lastStartX = (int)move[0] - 97;
	lastStartY = 56 - (int)move[1];
}

void restartEng()
{
	TerminateProcess(proc1.hProcess, 0);
	TerminateProcess(proc2.hProcess, 0);

	fenToBrd(StartPos,board1);
	
	startProcess(&eng1In,&eng1Out,EngPath1,&proc1);
	startProcess(&eng2In,&eng2Out,EngPath2,&proc2);
	
	int eCode;
	
	if (GetExitCodeProcess(proc1.hProcess,(LPDWORD)&eCode) && eCode == STILL_ACTIVE)
	{
		sprintf(textToEng,"uci\n");
		WriteFile(eng1In,textToEng,strlen(textToEng),(LPDWORD)&aTextLength,0);
		
		ZeroMemory(textFromEng,textFromPos);
		textFromPos = 0;
		while (1)
		{
			ReadFile(eng1Out,textFromEng + textFromPos,100000,(LPDWORD)&aTextLength,0);
			textFromPos += aTextLength;
			
			if (strstr(textFromEng,"uciok") && strstr(strstr(textFromEng,"uciok"),"\n")) break;
		}
	}
	
	if (GetExitCodeProcess(proc2.hProcess,(LPDWORD)&eCode) && eCode == STILL_ACTIVE)
	{
		sprintf(textToEng,"uci\n");
		WriteFile(eng2In,textToEng,strlen(textToEng),(LPDWORD)&aTextLength,0);
		
		ZeroMemory(textFromEng,textFromPos);
		textFromPos = 0;
		while (1)
		{
			ReadFile(eng2Out,textFromEng + textFromPos,100000,(LPDWORD)&aTextLength,0);
			textFromPos += aTextLength;
			
			if (strstr(textFromEng,"uciok") && strstr(strstr(textFromEng,"uciok"),"\n")) break;
		}
	}
	
	EngThinking = 0;
}

void resetGame()
{
	int responseStart = clock();
	
	if (needRestart)
	{
		needRestart = 0;
		restartEng();
		EngThinking = 0;
	}
	
	if (EngThinking)
	{
		WriteFile(turn == 1 ? eng1In : eng2In,"stop\n",5,0,0);
		while (1)
		{
			int byteCnt;
			PeekNamedPipe(turn == 1 ? eng1Out : eng2Out,0,0,0,(LPDWORD)&byteCnt,0);
			if (byteCnt == 0)
			{
				if (clock() > responseStart + 100)
				{
					restartEng();
					goto endEng;
				}
				continue;
			}
			
			ReadFile(turn == 1 ? eng1Out : eng2Out,textFromEng + textFromPos,100000,(LPDWORD)&aTextLength,0);
			textFromPos += aTextLength;
			
			bmLoc = strstr(textFromEng,"bestmove");
			if (bmLoc && strstr(bmLoc,"\n")) break;
		}
	}
	
	endEng:;
	
	EngThinking = 0;
	
	fenToBrd(StartPos,board1);
	castle = 0;
	turn = 1;
	isFlagged = 0;
	MoveTimeStart = -1;
	ClockW = sTime;
	ClockB = sTime;
	MoveNum = 1;
	ZeroMemory(MoveList,8192);
	MoveListPos = 0;
	GetAsyncKeyState(VK_LBUTTON);
	prClicked = 0;
}

void saveValue(const char* name,int val)
{
	HKEY key;
	
	RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\SPJTHKRZYCKJTCRLXVKXZHRKXPXNJGMMYJNBFNHG",0,0,0,KEY_WRITE,0,&key,0);

	RegSetValueEx(key,name,0,REG_DWORD,(LPBYTE)&val,4);

	RegCloseKey(key);
}

void saveString(const char* name,const char* str)
{
	HKEY key;
	
	RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\SPJTHKRZYCKJTCRLXVKXZHRKXPXNJGMMYJNBFNHG",0,0,0,KEY_WRITE,0,&key,0);

	RegSetValueEx(key,name,0,REG_SZ,(LPBYTE)str,strlen(str));

	RegCloseKey(key);
}

void loadValue(const char* name,int* val)
{
	HKEY key;

	RegOpenKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\SPJTHKRZYCKJTCRLXVKXZHRKXPXNJGMMYJNBFNHG",0,KEY_READ,&key);

	int valSize = 4;
	RegQueryValueEx(key,name,0,0,(LPBYTE)val,(LPDWORD)&valSize);

	RegCloseKey(key);
}

void loadString(const char* name,char* str)
{
	HKEY key;

	RegOpenKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\SPJTHKRZYCKJTCRLXVKXZHRKXPXNJGMMYJNBFNHG",0,KEY_READ,&key);

	int valSize = MAX_PATH;
	RegQueryValueEx(key,name,0,0,(LPBYTE)str,(LPDWORD)&valSize);

	RegCloseKey(key);
}

LRESULT CALLBACK cbSett(HWND hwnd,unsigned int message,WPARAM wparam,LPARAM lparam)
{
	if (message == WM_INITDIALOG) 
	{
		if (BotAutoNewGame) SendDlgItemMessage(hwnd,121,BM_SETCHECK,BST_CHECKED,0);
		if (AntiAl) SendDlgItemMessage(hwnd,122,BM_SETCHECK,BST_CHECKED,0);
		if (loseIfFlagged) SendDlgItemMessage(hwnd,123,BM_SETCHECK,BST_CHECKED,0);
		
		char timeText[50];
		sprintf(timeText,"%d",sTime/1000);
		SetDlgItemText(hwnd,124,timeText);
		
		SetDlgItemText(hwnd,125,EngPath1);
		
		SetDlgItemText(hwnd,126,EngPath2);
		
		needRestart = 0;
		
		return 1;
	}
	else if (message == WM_CLOSE) EndDialog(hwnd,0);
	else if (message == WM_COMMAND)
	{
		if (LOWORD(wparam) == 121) BotAutoNewGame = !BotAutoNewGame;
		else if (LOWORD(wparam) == 122) AntiAl = !AntiAl;
		else if (LOWORD(wparam) == 123) loseIfFlagged = !loseIfFlagged;
		else if (HIWORD(wparam) == EN_CHANGE)
		{
			if (LOWORD(wparam) == 124)
			{
				char timeText[50];
				GetDlgItemText(hwnd,124,timeText,sizeof(timeText));
				sscanf(timeText,"%d",&sTime);
				sTime *= 1000;
			}
			else if (LOWORD(wparam) == 125)
			{
				GetDlgItemText(hwnd,125,EngPath1,sizeof(EngPath1));
				needRestart = 1;
			}
			else if (LOWORD(wparam) == 126)
			{
				GetDlgItemText(hwnd,126,EngPath2,sizeof(EngPath2));
				needRestart = 1;
			}
		
			saveValue("1",BotAutoNewGame);
			saveValue("2",AntiAl);
			saveValue("3",loseIfFlagged);
			saveValue("4",sTime);
			saveString("5",EngPath1);
			saveString("6",EngPath2);
		}
	}
	
	return 0;
}

LRESULT CALLBACK cbGui(HWND hwnd,unsigned int message,WPARAM wparam,LPARAM lparam)
{
	if (message == WM_INITDIALOG) return 1;
	else if (message == WM_CLOSE) run = false;
	else if (message == WM_SIZE)
	{
		wWidth = LOWORD(lparam);
		wHeight = HIWORD(lparam);
	}
	else if (message == WM_KEYDOWN) 
	{
		printf("keypress\n");
	}
	else if (message == WM_COMMAND)
	{
		if (LOWORD(wparam) == 102) DialogBox(0,"settings",hwnd,(DLGPROC)&cbSett);
		if (LOWORD(wparam) == 101) 
		{
			char startPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 10";
			fenToBrd(startPos,board1);
			
			State = 1;
			resetGame();
		}
		if (LOWORD(wparam) == 103) 
		{
			char startPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 10";
			fenToBrd(startPos,board1);
			
			State = 2;
			prCol = 1;
			resetGame();
		}
		if (LOWORD(wparam) == 104) 
		{
			char startPos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 10";
			fenToBrd(startPos,board1);
			
			State = 2;
			prCol = 2;
			resetGame();
		}
		if (LOWORD(wparam) == 105) flipBrd = !flipBrd;
	}
	
	return 0;
}

int main()
{
	HWND hwndGui = CreateDialogParam(0,"gui",0,(DLGPROC)&cbGui,0);
	hdcGui = GetDC(hwndGui);
	hinsGui = GetModuleHandle(0);
	SetMenu(hwndGui,LoadMenu(hinsGui,"menu"));
	
	SetWindowPos(hwndGui,0,0,0,1100,800,SWP_NOMOVE|SWP_NOREPOSITION);
	
	biGui.bmiHeader.biSize = sizeof(biGui.bmiHeader);
	biGui.bmiHeader.biWidth = 640;
	biGui.bmiHeader.biHeight = 480;
	biGui.bmiHeader.biPlanes = 1;
	biGui.bmiHeader.biBitCount = 32;
	biGui.bmiHeader.biCompression = BI_RGB;
	
	int brdColors[4];
	hbmLoadBmps = (HBITMAP)LoadImage(hinsGui,"board",IMAGE_BITMAP,4,1,0);
	GetBitmapBits(hbmLoadBmps,4*1 * 4,brdColors);
	int* board = (int*)calloc(480*480,4);
	
	char fen1[128];
	char aMove[6] = "	\0";
	
	loadValue("1",&BotAutoNewGame);
	loadValue("2",&AntiAl);
	loadValue("3",&loseIfFlagged);
	loadValue("4",&sTime);
	loadString("5",EngPath1);
	loadString("6",EngPath2);
	
	ClockW = sTime;
	ClockB = sTime;
	
	restartEng();
	
	int* pieces[13];
	int loadPieces = -1;
	while (++loadPieces < 12)
	{
		pieces[loadPieces + 1] = (int*)calloc(60*60,4);
		
		char aPieceFileName[128];
		sprintf(aPieceFileName,"piece%d%d",loadPieces / 6,loadPieces % 6 + 1);
		hbmLoadBmps = (HBITMAP)LoadImage(hinsGui,aPieceFileName,IMAGE_BITMAP,60,60,0);
		GetBitmapBits(hbmLoadBmps,60*60 * 4,pieces[loadPieces + 1]);
	}
	
	hbmLoadBmps = (HBITMAP)LoadImage(hinsGui,"chars",IMAGE_BITMAP,1024,15,0);
	GetBitmapBits(hbmLoadBmps,1024*15 * 4,chars);
	
	drawRect(visualbg,640,0,0,640,480,0xFFFFFF);
	copyRect(visualbg,board,480,640,0,0,0,0,480,480,0x000000);
	
	while (run)
	{
		newFrame:;
		
		MSG msg;
		while (PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		memcpy(visual,visualbg,640*480 * 4);
		
		if (turn == 1 && ClockW <= 0)
		{
			ClockW = 1;
			if (State == 1 && BotAutoNewGame) aDelayStarted = clock();
			if (State == 1 || State == 2 && (prCol == 2 || loseIfFlagged))
			{
				State = 0;
				isFlagged = 1;
			}
		}
		else if (turn == 2 && ClockB <= 0)
		{
			ClockB = 1;
			if (State == 1 && BotAutoNewGame) aDelayStarted = clock();
			if (State == 1 || State == 2 && (prCol == 1 || loseIfFlagged))
			{
				State = 0;
				isFlagged = 2;
			}
		}
		
		if (State == 1)
		{
			if (!EngThinking)
			{
				toFen(fen1,board1,turn,castle,MoveNum);
				sprintf(textToEng,"position fen %s\ngo wtime %d btime %d\n",fen1,ClockW >= 0 ? ClockW : 1,ClockB >= 0 ? ClockB : 1);
				WriteFile(turn == 1 ? eng1In : eng2In,textToEng,strlen(textToEng),(LPDWORD)&aTextLength,0);
				
				ZeroMemory(textFromEng,textFromPos);
				textFromPos = 0;
				
				MoveTimeStart = clock();
			}
			
			while (1)
			{
				int byteCnt;
				PeekNamedPipe(turn == 1 ? eng1Out : eng2Out,0,0,0,(LPDWORD)&byteCnt,0);
				if (byteCnt == 0)
				{
					EngThinking = 1;
					goto endAct;
				}
				
				ReadFile(turn == 1 ? eng1Out : eng2Out,textFromEng + textFromPos,100000,(LPDWORD)&aTextLength,0);
				textFromPos += aTextLength;
				
				bmLoc = strstr(textFromEng,"bestmove");
				if (bmLoc && strstr(bmLoc,"\n")) break;
			}
			
			EngThinking = 0;
			
			memcpy(aMove,bmLoc + 9,5);
			
			if (aMove[0] == aMove[2] && aMove[1] == aMove[3]) 
			{
				State = 0;
				if (BotAutoNewGame) aDelayStarted = clock();
				goto newFrame;
			}
			
			memcpy(MoveList + MoveListPos,aMove,5);
			*(MoveList + MoveListPos + 5) = ' ';
			MoveListPos += 6;
			
			if (turn == 2) MoveNum += 1;
			
			playMove(board1,aMove);
			setLastMove(aMove);
			if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"a1") && (castle & 1) == 0) 
			{
				castle |= 1;
				if (*(int*)aMove == *(int*)"e1c1") playMove(board1,"a1d1");
			}
			if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"h1") && (castle & 2) == 0) 
			{
				castle |= 2;
				if (*(int*)aMove == *(int*)"e1g1") playMove(board1,"h1f1");
			}
			if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"a8") && (castle & 4) == 0) 
			{
				castle |= 4;
				if (*(int*)aMove == *(int*)"e8c8") playMove(board1,"a8d8");
			}
			if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"h8") && (castle & 8) == 0) 
			{
				castle |= 8;
				if (*(int*)aMove == *(int*)"e8g8") playMove(board1,"h8f8");
			}
			turn = 1 + turn % 2;
		}
		else if (State == 2)
		{
			if (turn == prCol)
			{
				if (GetAsyncKeyState(VK_LBUTTON) != 0)
				{
					while (GetAsyncKeyState(VK_LBUTTON) != 0) ;
					
					POINT mp; 
					GetCursorPos(&mp);
					ScreenToClient(hwndGui , &mp);
					
					mp.x = mp.x * 640 / wWidth;
					mp.y = mp.y * 480 / wHeight;
					if (mp.x < 0 || mp.x >= 480 || mp.y < 0 || mp.y >= 480) goto endAct;
					
					if (flipBrd)
					{
						mp.x = 480 - mp.x;
						mp.y = 480 - mp.y;
					}
					
					if (prClicked == 1)
					{
						prClicked = 0;
						aMove[0] = prClickedCol + 97;
						aMove[1] = 56 - prClickedRow;
						aMove[2] = mp.x * 8 / 480 + 97;
						aMove[3] = 56 - mp.y * 8 / 480;
						aMove[4] = aMove[3] == '8' && board1[prClickedCol + prClickedRow * 8] == 1 ? 'q' : ' ';
						
						prClickedCol = mp.x * 8 / 480;
						prClickedRow = mp.y * 8 / 480;
						
						if (aMove[0] == aMove[2] && aMove[1] == aMove[3]) ;
						else if (prCol == 1 && board1[prClickedCol + prClickedRow * 8] > 0 && board1[prClickedCol + prClickedRow * 8] <= 6 || prCol == 2 && board1[prClickedCol + prClickedRow * 8] > 6) prClicked = 1;
						else
						{
							playMove(board1,aMove);
							setLastMove(aMove);
							if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"a1") && (castle & 1) == 0) 
							{
								castle |= 1;
								if (*(int*)aMove == *(int*)"e1c1") playMove(board1,"a1d1");
							}
							if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"h1") && (castle & 2) == 0) 
							{
								castle |= 2;
								if (*(int*)aMove == *(int*)"e1g1") playMove(board1,"h1f1");
							}
							if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"a8") && (castle & 4) == 0) 
							{
								castle |= 4;
								if (*(int*)aMove == *(int*)"e8c8") playMove(board1,"a8d8");
							}
							if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"h8") && (castle & 8) == 0) 
							{
								castle |= 8;
								if (*(int*)aMove == *(int*)"e8g8") playMove(board1,"h8f8");
							}
							turn = 1 + turn % 2;
							
							printf("%s\n",aMove);
							
							memcpy(MoveList + MoveListPos,aMove,5);
							*(MoveList + MoveListPos + 5) = ' ';
							MoveListPos += 6;
						}
					}
					else
					{
						click:;
						
						prClicked = 1;
						prClickedCol = mp.x * 8 / 480;
						prClickedRow = mp.y * 8 / 480;
						if (board1[prClickedCol + prClickedRow * 8] == 0 || prCol == 1 && board1[prClickedCol + prClickedRow * 8] > 6 || prCol == 2 && board1[prClickedCol + prClickedRow * 8] <= 6) prClicked = 0;
					}
				
					drawnPrMove = 0;
				}
			}
			else if (turn != prCol && drawnPrMove >= 3)
			{
				if (!EngThinking)
				{
					toFen(fen1,board1,turn,castle,MoveNum);
					sprintf(textToEng,"position fen %s\ngo wtime %d btime %d\n",fen1,ClockW >= 0 ? ClockW : 1,ClockB >= 0 ? ClockB : 1);
					WriteFile(turn == 1 ? eng1In : eng2In,textToEng,strlen(textToEng),(LPDWORD)&aTextLength,0);
				
					ZeroMemory(textFromEng,textFromPos);
					textFromPos = 0;
					
					MoveTimeStart = clock();
				}
				
				while (1)
				{
					int byteCnt;
					PeekNamedPipe(turn == 1 ? eng1Out : eng2Out,0,0,0,(LPDWORD)&byteCnt,0);
					if (byteCnt == 0)
					{
						EngThinking = 1;
						goto endAct;
					}
				
					ReadFile(turn == 1 ? eng1Out : eng2Out,textFromEng + textFromPos,100000,(LPDWORD)&aTextLength,0);
					textFromPos += aTextLength;
					
					bmLoc = strstr(textFromEng,"bestmove");
					if (bmLoc && strstr(bmLoc,"\n")) break;
				}
				
				EngThinking = 0;
				
				memcpy(aMove,bmLoc + 9,5);
				
				printf("%s\n",aMove);
				
				if (aMove[0] == aMove[2] && aMove[1] == aMove[3]) 
				{
					MoveTimeStart = -1;
					State = 0;
					goto newFrame;
				}
				
				memcpy(MoveList + MoveListPos,aMove,5);
				*(MoveList + MoveListPos + 5) = ' ';
				MoveListPos += 6;
				
				MoveNum += 1;
				
				playMove(board1,aMove);
				setLastMove(aMove);
				if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"a1") && (castle & 1) == 0) 
				{
					castle |= 1;
					if (*(int*)aMove == *(int*)"e1c1") playMove(board1,"a1d1");
				}
				if ((*(short*)aMove == *(short*)"e1" || *(short*)aMove == *(short*)"h1") && (castle & 2) == 0) 
				{
					castle |= 2;
					if (*(int*)aMove == *(int*)"e1g1") playMove(board1,"h1f1");
				}
				if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"a8") && (castle & 4) == 0) 
				{
					castle |= 4;
					if (*(int*)aMove == *(int*)"e8c8") playMove(board1,"a8d8");
				}
				if ((*(short*)aMove == *(short*)"e8" || *(short*)aMove == *(short*)"h8") && (castle & 8) == 0) 
				{
					castle |= 8;
					if (*(int*)aMove == *(int*)"e8g8") playMove(board1,"h8f8");
				}
				turn = 1 + turn % 2;
				
				if (strstr(textFromEng,"ponder") == 0) 
				{
					MoveTimeStart = -1;
					State = 0;
					goto newFrame;
				}
			}
		}
		else if (State == 0 && aDelayStarted != 0 && clock() - aDelayStarted > AutoNGDelay) 
		{
			aDelayStarted = 0;
			State = 1;
			resetGame();
		}
		
		endAct:;
		
		if (MoveTimeStart != -1 && clock() > MoveTimeStart)
		{
			if (turn == 1) ClockW -= clock() - MoveTimeStart;
			else if (turn == 2) ClockB -= clock() - MoveTimeStart;
			MoveTimeStart = clock();
		}
		
		if (turn == 1) 
		{
			if (isFlagged == 1) drawRect(visual,640,480,0,80,20,0xAA0000);
			else drawRect(visual,640,480,0,80,20,0xFF8800);
			if (isFlagged == 2) drawRect(visual,640,560,0,80,20,0xAA0000);
			else drawRect(visual,640,560,0,80,20,0xCCCCCC);
		}
		else if (turn == 2)
		{
			if (isFlagged == 1) drawRect(visual,640,480,0,80,20,0xAA0000);
			else drawRect(visual,640,480,0,80,20,0xCCCCCC);
			if (isFlagged == 2) drawRect(visual,640,560,0,80,20,0xAA0000);
			else drawRect(visual,640,560,0,80,20,0xFF8800);
		}
		
		int aCTLen = 0;
		sprintf(aClockText,"%02d:%02d",ClockW / 60000,ClockW % 60000 / 1000);
		aCTLen = baLen(aClockText);
		writeTextRect(chars,aClockText,visual,640,495,2,aCTLen * 12,15,1);
		sprintf(aClockText,"%02d:%02d",ClockB / 60000,ClockB % 60000 / 1000);
		aCTLen = baLen(aClockText);
		writeTextRect(chars,aClockText,visual,640,575,2,aCTLen * 12,15,1);
		
		int LastToShow = MoveNum - 1 + turn / 2;
		
		int aMLShow = (LastToShow >= 6 ? LastToShow - 6 : 0);
		int aVisualLP = 0;
		while (++aMLShow <= LastToShow)
		{
			sprintf(aMLToShow,"%3d.",aMLShow % 1000);
			memcpy(aMLToShow + 4,MoveList + (aMLShow - 1) * 12,12);
			
			writeTextRect(chars,aMLToShow,visual,640,480,40 + aVisualLP,120,15,1);
			
			aVisualLP += 16;
		}
		
		int aDrawBrdX = 0,aDrawBrdY = 0;
		while (aDrawBrdY < 480)
		{
			drawRect(board,480,flipBrd ? 420 - aDrawBrdX : aDrawBrdX,flipBrd ? 420 - aDrawBrdY : aDrawBrdY,60,60,brdColors[(aDrawBrdX + aDrawBrdY) / 60 % 2]);
			
			aDrawBrdX += 60;
			if (aDrawBrdX >= 480)
			{
				aDrawBrdX = 0;
				aDrawBrdY += 60;
			}
		}
		
		if (MoveListPos != 0)
		{
			drawRect(board,480,(flipBrd ? 7 - lastX : lastX)*60,(flipBrd ? 7 - lastY : lastY)*60,60,60,brdColors[3]);
			drawRect(board,480,(flipBrd ? 7 - lastStartX : lastStartX)*60,(flipBrd ? 7 - lastStartY : lastStartY)*60,60,60,brdColors[3]);
		}
			
		if (prClicked)
		{
			drawRect(board,480,(flipBrd ? 7 - prClickedCol : prClickedCol)*60,(flipBrd ? 7 - prClickedRow : prClickedRow)*60,60,60,brdColors[2]);
		}
		
		copyRect(visual,board,480,640,0,0,0,0,480,480,0x000000);
		
		int aShowBrdX = 0,aShowBrdY = 0;
		while (aShowBrdY < 8)
		{
			if (board1[aShowBrdY * 8 + aShowBrdX] != 0) copyRect(visual,pieces[board1[aShowBrdY * 8 + aShowBrdX]],60,640,0,0,(flipBrd ? 7 - aShowBrdX : aShowBrdX) * 60,(flipBrd ? 7 - aShowBrdY : aShowBrdY) * 60,60,60,(board1[aShowBrdY * 8 + aShowBrdX] / 7 == 0 ? 0x000000 : 0xFFFFFF));
			
			aShowBrdX += 1;
			if (aShowBrdX >= 8)
			{
				aShowBrdX = 0;
				aShowBrdY += 1;
			}
		}
		
		drawnPrMove++;
		
		if (AntiAl)
		{
			blendRect(visuala,visual,640,640,0,0,0,0,640,480);
		}
		else
		{
			memcpy(visuala,visual,640*480 * 4);
		}
		
		StretchDIBits(hdcGui,0,wHeight,wWidth,-wHeight,0,0,640,480,visuala,&biGui,DIB_RGB_COLORS,SRCCOPY);
	}
	
	exit(0);
	
	return 0;
}
