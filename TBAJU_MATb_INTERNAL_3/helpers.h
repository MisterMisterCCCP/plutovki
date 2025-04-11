#pragma once
#include "pch.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <memory>
#include <iostream>

//DWORD = unsigned long (for addresses)


uintptr_t GetModuleBaseAddress(const wchar_t* moduleName);


class Hook {
private:
    void* toHook;                        // Adresse der zu hookenden Funktion
    std::unique_ptr<char[]> oldOpCodes;  // Speicher für die Original-Bytes
    int tLen;                             // Anzahl der überschriebenen Bytes
    bool enabled;                         // Ist der Hook aktiv?

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

struct Vector3
{
    char padlol[4];
    float x, y, z;
    float distance(const Vector3& other);
};

