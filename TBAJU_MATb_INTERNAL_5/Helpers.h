#pragma once
#include "pch.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <memory>

//DWORD = unsigned long (for addresses)


uintptr_t GetModuleBaseAddress(const wchar_t* moduleName);


class Hook {
private:
    void* toHook;
    std::unique_ptr<char[]> oldOpCodes;  
    int tLen;                             
    bool enabled;

public:
    Hook(void* toHook, void* ourFunc, int len);
    ~Hook();

    void enable();
    void disable();
    bool isEnabled();
};

class TrampHook
{
    void* gateWay;
    Hook* managedHook;
public:
    TrampHook(void* toHook, void* ourFunct, int len);
    ~TrampHook();
    void enable();
    void disable();
    bool isEnabled();
    void* getGateway();
};
