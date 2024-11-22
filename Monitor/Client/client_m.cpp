#include "client.h"
extern char bufferforpic[500000000];
int main()
{
    FreeConsole();
    ClientConnection thiscon("127.0.0.1");
    memset(bufferforpic,0,sizeof(bufferforpic));
    while(1)
    {
        if(thiscon.maySend)
        {
            Sleep(1500);
            thiscon.GetData();
            if(thiscon.SendData((void*)(thiscon.RetStr())))
            {
                thiscon.Restart();
            }
        }
    }
}