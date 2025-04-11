#include "pch.h"
#include "helpers.h"
uintptr_t GetModuleBaseAddress(const wchar_t* moduleName)
{
	return (uintptr_t)GetModuleHandleW(moduleName);
}

float Vector3::distance(const Vector3& other)
{
#ifdef _DEBUG
	std::cout << "Calc Distance Player X" << other.x << std::endl;
	std::cout << "Calc Distance Player Y" << other.y << std::endl;
	std::cout << "Calc Distance Player Z" << other.z << std::endl;
#endif
	return sqrt(
		(this->x - other.x) * (this->x - other.x) //x^2
		+ (this->y - other.y) * (this->y - other.y) //y^2
		+ (this->z - other.z) * (this->z - other.z) //z^2
	);
}


Hook::Hook(void* toHook, void* ourFunc, int len)
{
	this->toHook = toHook;
	this->oldOpCodes = nullptr;
	this->tLen = len;
	this->enabled = false;

	if (len < 5)
	{
		return;
	}
	DWORD currentProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &currentProtection);

	oldOpCodes = std::make_unique<char[]>(len);
	if (oldOpCodes != nullptr)
	{
		for (int i = 0; i < len; i++)
		{
			oldOpCodes[i] = ((char*)toHook)[i];
		}
	}
	memset(toHook, 0x90, len);
	DWORD relativeAdress = ((DWORD)ourFunc - (DWORD)toHook) - 5;
	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAdress;

	VirtualProtect(toHook, len, currentProtection, &currentProtection);

}

Hook::~Hook()
{
	if (oldOpCodes != nullptr)
	{
		DWORD currentProtection;
		VirtualProtect(toHook, tLen, PAGE_EXECUTE_READWRITE, &currentProtection);
		for (int i = 0; i < tLen; i++)
		{
			((char*)toHook)[i] = Hook::oldOpCodes[i];
		}
		VirtualProtect(toHook, tLen, currentProtection, &currentProtection);
	}

}

void Hook::enable()
{
	this->enabled = true;
}
void Hook::disable()
{
	this->enabled = false;
}
bool Hook::isEnabled()
{
	return enabled;
}


TrampHook::TrampHook(void* toHook, void* ourFunc, int len)
{
	this->gateWay = nullptr;
	this->managedHook = nullptr;

	gateWay = VirtualAlloc(NULL, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy_s(gateWay, len, toHook, len);
	uintptr_t returnAdress = (uintptr_t)toHook + len - (uintptr_t)gateWay - 5;
	*(BYTE*)((uintptr_t)gateWay + len) = 0xE9;
	*(DWORD*)((uintptr_t)gateWay + len + 1) = returnAdress;
	managedHook = new Hook(toHook, ourFunc, len);
}
TrampHook::~TrampHook()
{
	delete managedHook;
}
void TrampHook::enable()
{
	managedHook->enable();
}
void TrampHook::disable()
{
	managedHook->disable();
}
bool TrampHook::isEnabled()
{
	return managedHook->isEnabled();
}
void* TrampHook::getGateway()
{
	return gateWay;
}
