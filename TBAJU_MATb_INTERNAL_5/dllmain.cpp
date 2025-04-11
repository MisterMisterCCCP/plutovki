#include "pch.h"
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <gl/GL.h>
#include "Helpers.h"
#include "CustomGl.h"

#pragma comment(lib, "opengl32.lib")

static GLubyte colorA[3] = {255,0,0};
static GLubyte colorB[3] = {0,255,0};


static int tmpSize = 0;
static GL::Vector3 screenPos;
static GL::Vector3 tmpWrldPos;
static GL::Vector3 playerPos;
static float* W2S_Matrix;



typedef BOOL(__stdcall* twglSwapBuffers)(HDC hDc);
TrampHook* ESPHook;


// Strukturen für Vektoren & Spieler
struct Vector2Player { float x, y; };
struct Vector3Player { float x,y,z; };

class Player {
public:
    char pad_0000[4]; //0x0000
    Vector3Player headPos; //0x0004
    char pad_0010[24]; //0x0010
    Vector3Player playerPos; //0x0028
    Vector2Player viewAngles; //0x0034
    char pad_003C[176]; //0x003C
    int32_t health; //0x00EC
    char pad_00F0[1888]; //0x00F0
};

// ViewMatrix
float viewMatrix[16];


// Hook für `wglSwapBuffers`
BOOL __stdcall hwglSwapBuffers(HDC hDc) {
    Player** playerList = nullptr;
    int* playerListsize = nullptr;
    static uintptr_t baseAddress = GetModuleBaseAddress(L"ac_client.exe");
    playerList = *reinterpret_cast<Player***>(baseAddress + 0x0018AC04);
    playerListsize = reinterpret_cast<int*>(baseAddress + 0x0018AC0C);
    Player* ownPlayer = *reinterpret_cast<Player**>(baseAddress + 0x0017E254);
    W2S_Matrix = *reinterpret_cast<float**> (baseAddress + 0x000FB12C);
    #ifdef _DEBUG
    std::cout << "playerList Address: " << playerList << std::endl;
    std::cout << "playerListsize Address: " << playerListsize << std::endl;
    std::cout << "ownPlayer Address: " << ownPlayer << std::endl;
    std::cout << "W2S_Matrix Address: " << W2S_Matrix << std::endl;
    #endif


    GL::SetupOrtho();
    if(*playerListsize == tmpSize)
    {
    #ifdef _DEBUG
        std::cout << "INSIDE_DRAW_LOOP\n";
    #endif
        for(int i = 1; i < tmpSize; i++) //skipFirstPlayer
        {
        #ifdef _DEBUG
            std::cout << "CHECKING ENEMEY ADRESS: " << playerList[i] <<"\n";
            std::cout << "CHECKING ENEMEY HEALTH ADRESS: " << &playerList[i]->health << "\n";
            std::cout << "CHECKING ENEMEY HEALTH: " << playerList[i]->health << "\n";
        #endif
            if(playerList[i]->health > 0 && playerList[i]->health < 200)
            {
                #ifdef _DEBUG
                std::cout << "INSIDE_DRAWING\n";
                #endif

                tmpWrldPos.x = playerList[i]->playerPos.x;
                tmpWrldPos.y = playerList[i]->playerPos.y;
                tmpWrldPos.z = playerList[i]->playerPos.z;
                std::cout << "CHECKING ENEMEY PLAYER POS X ADRESS: " << &playerList[i]->playerPos.x << "\n";
                std::cout << "CHECKING OWN PLAYER POS X ADRESS: " << &ownPlayer->playerPos.x << "\n";
                std::cout << "CHECKING OWN PLAYER POS X: " << ownPlayer->playerPos.x << "\n";
                std::cout << "CHECKING OWN PLAYER POS Y: " << ownPlayer->playerPos.y << "\n";
                std::cout << "CHECKING OWN PLAYER POS Z: " << ownPlayer->playerPos.z << "\n";

                playerPos.x = ownPlayer->playerPos.x;
                playerPos.y = ownPlayer->playerPos.y;
                playerPos.z = ownPlayer->playerPos.z;

                float distance = playerPos.distance(tmpWrldPos);
                bool vgl = GL::WorldToScreen(tmpWrldPos, screenPos, W2S_Matrix);
                #ifdef _DEBUG
                std::cout << "CHECKING DISTANCE: " << distance << "\n";
                std::cout << "CHECKING WORLD_TO_SCREEN: " << vgl << "\n";
                std::cout << "CHECKING VIEW MATRIX ADRESS: " << W2S_Matrix << "\n";
                #endif

                if(distance > 5.0f && GL::WorldToScreen(tmpWrldPos, screenPos, W2S_Matrix))
                {
                        GL::DrawESPBox(screenPos.x, screenPos.y, distance, colorA, playerList[i]->health);
                }

            }
            #ifdef _DEBUG
            std::cout << "OUTSIDE_DRAWING\n";
            #endif
        
        }
    
    }
    else
    {
        tmpSize = *playerListsize;
    #ifdef _DEBUG
        std::cout << "OUTSIDE_DRAW_LOOP";
    #endif
    }
    GL::RestoreGl();
    return ((twglSwapBuffers)ESPHook->getGateway())(hDc);
}

// Hauptfunktion
DWORD WINAPI funnymain(HMODULE hMod) {
#ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Debug-Konsole aktiviert!\n";
#endif

    HMODULE openGL = GetModuleHandleA("opengl32.dll");
    void* swapBuffersAdress = GetProcAddress(openGL, "wglSwapBuffers");
    ESPHook = new TrampHook(swapBuffersAdress, hwglSwapBuffers, 5);

    while (!GetAsyncKeyState(VK_END)) {
        Sleep(50);
    }
    delete ESPHook;

#ifdef _DEBUG
    if (f != nullptr) fclose(f);
    FreeConsole();
#endif

    FreeLibraryAndExitThread(hMod, 0);
    return 0;
}

// DLL-Einstiegspunkt
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        HANDLE tHandle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)funnymain, hModule, 0, 0);
        if (tHandle) CloseHandle(tHandle);
        else return FALSE;
    }
    return TRUE;
}
//*

