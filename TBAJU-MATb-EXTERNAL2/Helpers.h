#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

DWORD GetProcId(const wchar_t* procName);

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* moduleName);

uintptr_t ResolveDynamicAdress(HANDLE gameHandle, uintptr_t ptr, std::vector<unsigned int> offsets);

void PatchExternal(HANDLE gameHandle, uintptr_t dst, BYTE* src, size_t size);

class NopExternal 
{
private:
	HANDLE gameHandle;
	uintptr_t targetAdress;
	size_t size;
	BYTE* originalCode;
	BYTE* nopCode;

public:
	NopExternal(HANDLE gameHandle, uintptr_t targetAdress, size_t size);
	~NopExternal();
	void enable();
	void disable();
};