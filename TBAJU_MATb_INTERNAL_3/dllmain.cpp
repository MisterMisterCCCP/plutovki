#include "pch.h"
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include "helpers.h"
#include "math.h"


#define M_PI 3.14159265358979323846




typedef BOOL(__stdcall* twglSwapBuffers)(HDC hDc);
TrampHook* ESPHook;


// Strukturen für Vektoren & Spieler
struct Vector2Player { float x, y; };
struct Vector3Player { float x, y, z; };

class Player3 {
public:
    char padlol[4];
    Vector3Player headPos;
    char pad[24];
    Vector3Player playerPos;
    Vector2Player viewAngles;
    char anotherPad[176];
    int health;
};

uintptr_t baseAddress = GetModuleBaseAddress(L"ac_client.exe");
Player3** playerList = *reinterpret_cast<Player3***>(baseAddress + 0x0018AC04);
int* playerListSize = reinterpret_cast<int*>(baseAddress + 0x0018AC0C);
Player3* ownPlayer = *reinterpret_cast<Player3**>(baseAddress + 0x0017E254);
Player3* bestTarget = nullptr;
int tmpSize = 0;
float bestDistance = 0;


// Hook für `wglSwapBuffers`
void aimbot() 
{
#ifdef _DEBUG
    std::cout << "playerList Address: " << playerList << std::endl;
    std::cout << "playerListsize Address: " << playerListSize << std::endl;
    std::cout << "ownPlayer Address: " << ownPlayer << std::endl;
#endif

        // select the closest player that can be seen as a target.
        for (int i = 1; i < tmpSize; i++)
        {
        #ifdef _DEBUG
            std::cout << "Adress of Player X: " << playerList[i] << std::endl;
            std::cout << "Adress of Player X Head Pos X: " << &playerList[i]->headPos.x << " " << playerList[i]->headPos.x << std::endl;
            std::cout << "Adress of Player X Head Pos Y: " << &playerList[i]->headPos.y << " " << playerList[i]->headPos.y << std::endl;
            std::cout << "Adress of Player X Head Pos Z: " << &playerList[i]->headPos.z << " " << playerList[i]->headPos.z << std::endl;
            std::cout << "Adress of ownPlayer : " << ownPlayer << std::endl;
            std::cout << "Adress of ownPlayer Head Pos X: " << &ownPlayer->headPos.x << " " << ownPlayer->headPos.x << std::endl;
            std::cout << "Adress of ownPlayer Head Pos Y: " << &ownPlayer->headPos.y << " " << ownPlayer->headPos.y << std::endl;
            std::cout << "Adress of ownPlayer Head Pos Z: " << &ownPlayer->headPos.z << " " << ownPlayer->headPos.z << std::endl;
        #endif
                float tmpDistance = reinterpret_cast<Vector3*>(ownPlayer)
                    ->distance(*((Vector3*)playerList[i]));

                if (bestTarget)
                {
                #ifdef _DEBUG
                    std::cout << "INSIDE 1 IF CLAUSE" << std::endl;
                #endif
                    if (tmpDistance < bestDistance)
                    {
                    #ifdef _DEBUG
                        std::cout << "INSIDE 2 IF CLAUSE" << std::endl;
                    #endif
                        bestTarget = playerList[i];
                        bestDistance = tmpDistance;
                    }
                }
                else
                {
                #ifdef _DEBUG
                    std::cout << "INSIDE 1 ELSE CLAUSE" << std::endl;
                #endif
                    bestTarget = playerList[i];
                    bestDistance = tmpDistance;
                }
        }

        if (bestTarget && ownPlayer->health > 0 && ownPlayer->health < 200 && !isnan(ownPlayer->headPos.z))
        {
        #ifdef _DEBUG
            std::cout << "INSIDE 3 IF CLAUSE... CALCULATE VIEW" << std::endl;
        #endif
                float dx = bestTarget->headPos.x - ownPlayer->headPos.x;
                float dy = bestTarget->headPos.y - ownPlayer->headPos.y;
                float dz = bestTarget->headPos.z - ownPlayer->headPos.z;
                float localDistance = sqrt(dx * dx + dy * dy);
                ownPlayer->viewAngles.x = -atan2f(dx, dy) / M_PI * 180.f + 180.f;// yaw (Links rechts)
                ownPlayer->viewAngles.y = asinf(dz/bestDistance) * 180 / M_PI; //pitch (oben unten)       
        }
        else
        {
        #ifdef _DEBUG
            std::cout << "INSIDE 2 ELSE CLAUSE" << std::endl;
        #endif
            tmpSize = *playerListSize;
        }
}

// Hauptfunktion
DWORD WINAPI funnymain(HMODULE hMod) {
#ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Debug-Konsole aktiviert!\n";
#endif



    while (!GetAsyncKeyState(VK_END)) {
        aimbot();

    }

#ifdef _DEBUG
    if (f != nullptr) fclose(f);
    FreeConsole();
#endif
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



