#include "server.h"
char bufferforpic[500000000];
//std::vector<std::thread*> AllClients;
std::vector<SOCKET> AllClients;
std::vector<sockaddr_in> AllClAddr;    
SOCKET ListenSocket;

bool begin=0;
int curask=0;

int main()
{
    WSADATA wsaData;
    sockaddr_in service;
    memset(bufferforpic,0,sizeof(bufferforpic));
    int i=0;
    service.sin_family = AF_INET;
    service.sin_port = htons(27015);
    inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);

    if(WsaStart(&wsaData)) return 0;
    if(SockListen(ListenSocket)) return 0;
    if(Binding(ListenSocket,(SOCKADDR*)&service)) return 0;
    if(Listening(ListenSocket)) return 0;

    // Create a SOCKET for accepting incoming requests.
    std::cout<<"Waiting for client to connect..."<<std::endl;

    std::thread AcceptionThread(ClientWork);
    std::thread WindowProc(WinInit);

    NewAccept(&ListenSocket);

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}

void ClientWork()
{
    std::string out;
    
    ClWorAg:
    int iResult=0;
    for(int i=0;i<AllClients.size();i++)
        {
            char buffer[1000000];
            char number[10];
            iResult=recv(AllClients[i],buffer,100000,0);
            if ( iResult > 0 ) {out+=itoa(i,number,10);out+="| ";out+=buffer;}
            else 
            {
                std::cout<<"recv failed: "<< WSAGetLastError()<<std::endl;
                closesocket(AllClients[i]);
                AllClients.erase(AllClients.begin()+i);
                AllClAddr.erase(AllClAddr.begin()+i);
                std::cout<<"Client disabled"<<std::endl;
            }
            memset(buffer,0,strlen(buffer));
        }
    system("cls");
    std::cout<<out;
    Sleep(1000);
    out.clear();

    goto ClWorAg;
}

int NewAccept(SOCKET* ListenSocket)
{
    while(true)
    {
        fd_set rds;
        FD_ZERO(&rds);
        FD_SET(*ListenSocket, &rds);

        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        select(*ListenSocket+1,&rds,0,0,&timeout);
        sockaddr_in newaddr;
        int size =sizeof(newaddr);
        SOCKET newsock=accept(*ListenSocket, (sockaddr *)&newaddr, &size);
    
        if (newsock == INVALID_SOCKET) 
        {
            std::cout<<"accept failed with error: "<< WSAGetLastError()<<std::endl;
            closesocket(*ListenSocket);
            WSACleanup();
            return 0;
        } 
        AllClAddr.push_back(newaddr);
        AllClients.push_back(newsock);
    }
    return 0;
}

int SockListen(SOCKET&Socket)
{
    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == INVALID_SOCKET) 
    {
        std::cout<<"socket failed with error: "<< WSAGetLastError()<<std::endl;
        WSACleanup();
        return 1;
    }
    return 0;
}

int WsaStart(LPWSADATA wsaData)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), wsaData);
    if (iResult != NO_ERROR) 
    {
        std::cout<<"WSAStartup failed with error: "<< iResult<<std::endl;
        return 1;
    }
    return 0;
}

int Binding(SOCKET&sock, SOCKADDR* addr)
{
     if (bind(sock, addr, sizeof (*addr)) == SOCKET_ERROR) 
    {
        std::cout<<"bind failed with error:"<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    return 0;
}

int Listening(SOCKET& sock)
{
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr<<"listen failed with error: "<< WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    return 0;
}

void WinInit()
{
HINSTANCE hInstance =GetModuleHandle(nullptr);
WNDCLASSA wc = { };
wc.lpfnWndProc   = WindowProc;
wc.hInstance     = hInstance;
wc.lpszClassName = "CLASS";
RegisterClassA(&wc);

HWND hwnd = CreateWindowExA(0, "CLASS", "Monitor", WS_SYSMENU , 0, 0, 
    220, 120, NULL, NULL, hInstance, NULL);

W_Edit=CreateWindowA("EDIT","",WS_BORDER | WS_CHILD,0,10,200,30,hwnd,0,hInstance,0);
W_Button=CreateWindowA("BUTTON","GetScreenShot",WS_BORDER | WS_CHILD,0,50,200,30,hwnd,0,hInstance,0);

if (hwnd == NULL)  
{
    std::cerr<<"Window failed with error: "<< GetLastError()<<std::endl;
    return ;
}
if (W_Edit == NULL) std::cerr<<"Edit failed with error: "<< GetLastError()<<std::endl;


HFONT font=CreateFont(26,20,0,0,4,0,0,0,0,0,0,0,5,0);
SendMessage(W_Edit, WM_SETFONT, (WPARAM)font, 0);

ShowWindow(hwnd,5);
ShowWindow(W_Edit,5);
ShowWindow(W_Button,5);

MSG msg;
BOOL bRet;
while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
{ 
    if (bRet == -1)
        PostQuitMessage(0);
    else
    {
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }
}
return;
}

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
PAINTSTRUCT ps;
int par=0;
 switch (uMsg)
    { 
    case WM_DESTROY: ExitProcess(0);
    case WM_COMMAND:
    if(lParam==(long long)W_Button)
       {
            char output[100];
            GetWindowTextA(W_Edit,output,10);
            AscScreen(atoi(output));
       } break;
     default: break;
	}
    return DefWindowProcA(hwnd,uMsg,wParam,lParam);
}

void AscScreen(int number)
{
    SOCKET CurAsk = AllClients[number];
    sockaddr_in curaddr= AllClAddr[number];
    char message[]={SCREENSHOT};
    u_short port=CLIENTGETPORT;
    curaddr.sin_port=htons(port);

    SOCKET ConnectSocket;
    ConnectSocket = socket(2,1,6);

    int res=connect(ConnectSocket,(sockaddr*)&curaddr,sizeof(curaddr));
    while(res==SOCKET_ERROR)
    {
        port++;
        curaddr.sin_port=htons(port);
        res=connect(ConnectSocket,(sockaddr*)&curaddr,sizeof(curaddr));
    }

    res=send(ConnectSocket,message,1,0);
    if(res==SOCKET_ERROR)
    {
        std::cerr<<"No sending ask"<<WSAGetLastError(); 
        return;
    }
  
    closesocket(ConnectSocket);
    
    ConnectSocket = socket(2,1,6);
    curaddr.sin_family = AF_INET;
    curaddr.sin_port = htons(GETPICPORT);
    inet_pton(AF_INET, "127.0.0.1", &curaddr.sin_addr);

    res=bind(ConnectSocket,(sockaddr *)&curaddr,sizeof(curaddr));
    if(res==SOCKET_ERROR)
    {
        std::cerr<<"No binding bmp "<<WSAGetLastError(); 
        return;
    }
    listen(ConnectSocket,1);
    
    sockaddr_in newaddr;
    int size = sizeof(newaddr);
    SOCKET picfrom=accept(ConnectSocket, (sockaddr *)&newaddr, &size);

    if (picfrom == INVALID_SOCKET)
    {
        std::cerr<<"No accepting bmp "<<WSAGetLastError(); 
        return;
    }
    Sleep(300);
    res=recv(picfrom,bufferforpic,sizeof(bufferforpic),0);
    if(res==SOCKET_ERROR)
    {
        std::cout<<"No recieving bmp"<<WSAGetLastError();
        return; 
    }

    OFSTRUCT openf;
    HFILE ToRead=OpenFile("Recieve.bmp",&openf,OF_WRITE | OF_CREATE);
    WriteFile((HANDLE)ToRead,bufferforpic,res,0,0);
    CloseHandle((HANDLE)ToRead);

    closesocket(picfrom);
    closesocket(ConnectSocket);
}