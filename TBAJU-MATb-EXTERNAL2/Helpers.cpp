#include "Helpers.h"



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

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* moduleName)
{
	uintptr_t modBaseAddress = 0;
	HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

	if (moduleSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(entry);
		if (Module32First(moduleSnapshot, &entry))
		{
			do
			{
				if (!_wcsicmp(entry.szModule, moduleName))
				{
					modBaseAddress = (uintptr_t)entry.modBaseAddr;
					break;
				}

			} while (Module32Next(moduleSnapshot, &entry));

		}
	}
	CloseHandle(moduleSnapshot);
	return modBaseAddress;
}

uintptr_t ResolveDynamicAdress(HANDLE gameHandle, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	for(unsigned int i = 0; i < offsets.size(); i++)
	{
		ReadProcessMemory(gameHandle, LPCVOID(ptr), &ptr, sizeof(ptr), 0);
		ptr += offsets[i];
	}
	return ptr;
}

void PatchExternal(HANDLE gameHandle, uintptr_t dst, BYTE* src, size_t size)
{
	DWORD oldProtection;
	VirtualProtectEx(gameHandle, (LPVOID)dst, size, PAGE_EXECUTE_READWRITE, &oldProtection);
	WriteProcessMemory(gameHandle, (LPVOID)dst, src, size, nullptr);
	VirtualProtectEx(gameHandle, (LPVOID)dst, size, oldProtection, &oldProtection);
}

NopExternal::NopExternal(HANDLE gameHandle, uintptr_t targetAdress, size_t size) {
	this->gameHandle = gameHandle;
	this->targetAdress = targetAdress;
	this->size = size;

	this->originalCode = new BYTE[size];  // Speicher reservieren
	this->nopCode = new BYTE[size];       // Speicher reservieren

	memset(this->nopCode, 0x90, size);  // NOPs setzen
	ReadProcessMemory(gameHandle, (LPCVOID)targetAdress, this->originalCode, size, 0);
}

NopExternal::~NopExternal() {
	delete[] this->originalCode;
	delete[] this->nopCode;
}

void NopExternal::enable() {
	PatchExternal(gameHandle, targetAdress, nopCode, size);
}

void NopExternal::disable() {
	PatchExternal(gameHandle, targetAdress, originalCode, size);
}

