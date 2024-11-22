#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>

#define SCREENSHOT 0x15
#define CLIENTGETPORT 27016
#define GETPICPORT  25000

int WsaStart(LPWSADATA);
int SockListen(SOCKET&);
int Binding(SOCKET&, SOCKADDR*);
int Listening(SOCKET&);
int NewAccept(SOCKET*);
void ClientWork();
void GetScreen(int i);
void WinInit();
LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM ,LPARAM); 
void AscScreen(int number);

HWND W_Edit;
HWND W_Button;