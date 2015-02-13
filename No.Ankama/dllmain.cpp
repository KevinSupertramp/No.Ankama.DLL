// dllmain.cpp : Définit le point d'entrée pour l'application DLL.
#include "stdafx.h"
#include "detours.h"
#include <ws2tcpip.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "detours.lib")

int (WINAPI *Real_Connect)(SOCKET s, const struct sockaddr *name, int namelen) = connect;
int WINAPI Mine_Connect(SOCKET s, const struct sockaddr *name, int namelen);

FILE* logFile;

BOOL WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID &)Real_Connect, Mine_Connect);
            DetourTransactionCommit();
            break;

        case DLL_PROCESS_DETACH:
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID &)Real_Connect, Mine_Connect);
            DetourTransactionCommit(); 
            break;
    }

    return TRUE;
}

int WINAPI Mine_Connect(SOCKET s, const struct sockaddr *name, int namelen)
{
    char* ip;
    struct sockaddr_in sa;
    std::vector<char*> listIP;
    bool isAnkamaLogin = false;

    ip = inet_ntoa(((struct sockaddr_in*)name)->sin_addr);

	fopen_s(&logFile, "No.Ankama.Log.txt", "a+");
	fprintf(logFile, "%s\n", ip);
	fclose(logFile);

	// Ankama servers list
    listIP.push_back("213.248.126.37");
    listIP.push_back("213.248.126.38");
    listIP.push_back("213.248.126.39");
    listIP.push_back("213.248.126.40");

    for(unsigned int i = 0; i < listIP.size(); i++)
    {  
        if((strcmp(ip, listIP[i]) == 0))
            isAnkamaLogin = true;
    }
	
    if(isAnkamaLogin)
    {
		// The IP and port you want to redirect client to
        sa.sin_addr.s_addr = inet_addr("127.0.0.1");
        sa.sin_port = htons(443);
        sa.sin_family = AF_INET;

        return Real_Connect(s, (sockaddr*)&sa, sizeof(sa));
    }

    return Real_Connect(s, name, namelen);
}

extern "C" __declspec(dllexport) void dummy(void){
    return;
}