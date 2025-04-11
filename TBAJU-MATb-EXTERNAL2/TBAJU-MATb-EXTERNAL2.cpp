// TBAJU-MATb-EXTERNAL2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include "Helpers.h"

int main()
{
    wchar_t gameName[] = L"ac_client.exe";
    uintptr_t playerOffset = 0x0017E254;

    // Get process ID and module base address
    DWORD procId = GetProcId(gameName);
    if (!procId) {
        std::cerr << "Error: Process not found!\n";
        return 1;
    }

    uintptr_t moduleBaseAdress = GetModuleBaseAddress(procId, gameName);
    if (!moduleBaseAdress) {
        std::cerr << "Error: Module base address not found!\n";
        return 1;
    }

    uintptr_t playerBaseAdress = moduleBaseAdress + playerOffset;

    // Open process handle
    HANDLE gameHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!gameHandle) {
        std::cerr << "Error: OpenProcess failed! Error Code: " << GetLastError() << std::endl;
        return 1;
    }



    // ? Correct calculation of addresses
    std::vector<unsigned int> offsetsHealthAdress = {0xEC};
    std::vector<unsigned int> offsetsSniperClip = {0x13C};
    std::vector<unsigned int> offsetsSniperTotal = {0x118};

    uintptr_t healthAdress = ResolveDynamicAdress(gameHandle, playerBaseAdress, offsetsHealthAdress);
    uintptr_t sniperClipAmmoAdress = ResolveDynamicAdress(gameHandle, playerBaseAdress, offsetsSniperClip);
    uintptr_t sniperTotalAmmoAdress = ResolveDynamicAdress(gameHandle, playerBaseAdress, offsetsSniperTotal);
    uintptr_t sniperRapidFireAdress = 0x004C73EA;

    int lastHealth = 0;
    int lastSniperClipAmmo = 0;
    int lastSniperTotalAmmo = 0;
    int hackedValue = 65000;
    DWORD exitCode = 0;

    std::cout << "TBAJU-MAT-EXTERNAL started! Use the following hotkeys:\n";
    std::cout << "[NUMPAD1] - Enable hack\n";
    std::cout << "[NUMPAD2] - Disable hack\n";
    std::cout << "[NUMPAD3] - Exit program\n";
    NopExternal* rapid = new NopExternal(gameHandle, sniperRapidFireAdress, 2);

    while (GetExitCodeProcess(gameHandle, &exitCode) && exitCode == STILL_ACTIVE)
    {
        if (GetAsyncKeyState(VK_NUMPAD1) & 0x1)
        {
            std::cout << "NUMPAD1 pressed - Hack activated!\n";
            ReadProcessMemory(gameHandle, (LPVOID)healthAdress, &lastHealth, sizeof(lastHealth), nullptr);
            ReadProcessMemory(gameHandle, (LPVOID)sniperClipAmmoAdress, &lastSniperClipAmmo, sizeof(lastSniperClipAmmo), nullptr);
            ReadProcessMemory(gameHandle, (LPVOID)sniperTotalAmmoAdress, &sniperTotalAmmoAdress, sizeof(lastSniperTotalAmmo), nullptr);
            WriteProcessMemory(gameHandle, (LPVOID)healthAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, (LPVOID)sniperClipAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, (LPVOID)sniperTotalAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);
            rapid->enable();
        }
        if(GetAsyncKeyState(VK_NUMPAD2) & 0x1)
        {
            WriteProcessMemory(gameHandle, (LPVOID)healthAdress, &lastHealth, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, (LPVOID)sniperClipAmmoAdress, &lastSniperClipAmmo, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, (LPVOID)sniperTotalAmmoAdress, &lastSniperTotalAmmo, sizeof(hackedValue), nullptr);
            rapid->disable();
        
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 0x1)
        {
            std::cout << "NUMPAD3 pressed - Program exited.\n";
            delete rapid; 
            CloseHandle(gameHandle);
            break;
        }
    }

    return 0;
}
















/*
#include <iostream>
#include <windows.h>
#include "Helpers.h"





int main()
{
    wchar_t gameName[] = L"ac_client.exe";
    size_t playerOffset = 0x0017E254;

    DWORD procId = GetProcId(gameName);
    uintptr_t moduleBaseAdress = GetModuleBaseAddress(procId, gameName);

    uintptr_t playerBaseAdress = moduleBaseAdress + playerOffset;

    HANDLE gameHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

    BYTE* playerAdress;

    ReadProcessMemory(gameHandle, (BYTE*)playerBaseAdress, &playerAdress, sizeof(playerAdress), nullptr);

    //std::cout << "Player Base Address: 0x" << std::hex << playerBaseAdress << std::endl;
    //std::cout << "Player Address: 0x" << std::hex << playerAdress << std::endl;

    BYTE* healthAdress = playerAdress + 0xEC;
    BYTE* sniperClipAmmoAdress = playerAdress + 0xEC;
    BYTE* sniperTotalAmmoAdress = playerAdress + 0xEC;

    int originalHealthValue = 100;
    int originalSniperClipAmmoValue = 5;
    int originalSniperTotalAmmoValue = 5;

    int hackedValue = 65000;
    DWORD exitCode = 0;

    bool hackEnabled = false;

    std::cout << "Hello from TBAJU-MAT-EXTERNAL check the listet hotkeys below for the possible actions\n";


    while(GetExitCodeProcess(gameHandle, &exitCode) && exitCode == STILL_ACTIVE)
    {
        if(GetAsyncKeyState(VK_NUMPAD1) & 0x1)
        {
            std::cout << "pressed 1";
            WriteProcessMemory(gameHandle, healthAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, sniperClipAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, sniperTotalAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);

        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 0x1)
        {
            std::cout << "pressed 2";
            WriteProcessMemory(gameHandle, healthAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, sniperClipAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);
            WriteProcessMemory(gameHandle, sniperTotalAmmoAdress, &hackedValue, sizeof(hackedValue), nullptr);
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 0x1)
        {
            std::cout << "pressed 3";
            CloseHandle(gameHandle);
            break;
        }

    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

*/