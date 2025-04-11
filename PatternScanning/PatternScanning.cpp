#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <cstring>

bool DataCompare(const BYTE* data, const BYTE* pattern, const char* mask)
{
    for (; *mask; ++mask, ++data, ++pattern)
    {
        if (*mask == 'x' && *data != *pattern)
            return false;
    }
    return true;
}


std::vector<uintptr_t> PatternScan(HANDLE hProcess, uintptr_t startAddress, const BYTE* pattern, const char* mask)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uintptr_t scanStart = (startAddress == 0) ? reinterpret_cast<uintptr_t>(sysInfo.lpMinimumApplicationAddress) : startAddress;
    uintptr_t endAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);
    SIZE_T patternLength = strlen(mask);
    std::vector<uintptr_t> foundAddresses;

    MEMORY_BASIC_INFORMATION mbi;

    for (uintptr_t currentAddress = scanStart; currentAddress < endAddress; currentAddress += mbi.RegionSize)
    {
        if (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(currentAddress), &mbi, sizeof(mbi)) == 0)
            continue;

        if (mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_NOACCESS) || (mbi.Protect & PAGE_GUARD))
            continue;

        SIZE_T bytesRead;
        std::vector<BYTE> buffer(mbi.RegionSize);

        if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead))
        {
            uintptr_t baseAddress = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            SIZE_T scanOffset = (scanStart > baseAddress) ? (scanStart - baseAddress) : 0;

            for (SIZE_T i = scanOffset; i <= bytesRead - patternLength; i++)
            {
                if (DataCompare(buffer.data() + i, pattern, mask))
                {
                    uintptr_t foundAddress = baseAddress + i;
                    foundAddresses.push_back(foundAddress);
                }
            }
        }
    }
    return foundAddresses;
}


// Holt die Prozess-ID anhand des Namens
DWORD GetProcId(const wchar_t* procName)
{
    DWORD procId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(entry);
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (!_wcsicmp(entry.szExeFile, procName))
                {
                    procId = entry.th32ProcessID;
                    break;
                }

            } while (Process32Next(snapshot, &entry));
        }
    }
    CloseHandle(snapshot);
    return procId;
}

// Hauptprogramm
int main()
{
    DWORD procId = GetProcId(L"ac_client.exe");
    if (procId == 0)
    {
        std::cerr << "Prozess nicht gefunden!" << std::endl;
        return 1;
    }

    HANDLE gHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    if (!gHandle)
    {
        std::cerr << "Fehler beim Öffnen des Prozesses!" << std::endl;
        return 1;
    }

    BYTE pattern[] = { 0xB4, 0xD0, 0x54, 0x00, 0x64,0x00 };
    const char* mask = "xxxx?x";
    std::vector<uintptr_t> addresses = PatternScan(gHandle, 0, pattern, mask);
    uintptr_t healthadr = addresses[0] + 4;
    int health = 0;
    if (!addresses.empty())
    {
        std::cout << "Pattern gefunden an folgenden Adressen:" << std::endl;
        for (uintptr_t addr : addresses)
        {
            std::cout << "0x" << std::hex << addr << std::endl;
        }
    }
    else
    {
        std::cout << "Pattern nicht gefunden." << std::endl;
    }

    DWORD exitCode = 0;
    while (GetExitCodeProcess(gHandle, &exitCode) && exitCode == STILL_ACTIVE)
    {
        ReadProcessMemory(gHandle, (LPVOID)healthadr, &health, sizeof(int), NULL);
        Sleep(20);
        std::cout << "Health Value is: " << std::dec << health << "\n";

        if (GetAsyncKeyState(VK_NUMPAD2) & 0x1)
        {
            break;
        }
    }
    CloseHandle(gHandle);
    return 0;
}
