// anti_debug.cpp - Implementierung der Anti-Debugging-Techniken
#include "anti_debug.h"
#include "common.h"
#include <iostream>

//----------------------------------------------------------------------------
// Definitionen für undokumentierte Windows-API-Funktionen und Konstanten
//----------------------------------------------------------------------------

// Konstanten für Windows-API-Aufrufe
#define ThreadHideFromDebugger 0x11
#define ProcessDebugPort 7
#define SystemKernelDebuggerInformation 35

// Funktionstypen für dynamisches Laden aus ntdll.dll
typedef NTSTATUS(NTAPI* NtQueryInformationProcessPtr)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI* NtQuerySystemInformationPtr)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI* NtSetInformationThreadPtr)(
    HANDLE ThreadHandle,
    ULONG ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength
    );

// Struktur für Kernel-Debugger-Informationen
typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
    BOOLEAN KernelDebuggerEnabled;
    BOOLEAN KernelDebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, * PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

/**
 * Konstruktor: Initialisiert den AntiDebug-Monitor.
 */
AntiDebug::AntiDebug() : isRunning(false), hMonitorThread(NULL) {
    // Keine weiteren Initialisierungen notwendig
}

/**
 * Destruktor: Stellt sicher, dass der Überwachungsthread beendet wird.
 */
AntiDebug::~AntiDebug() {
    Stop();
}

/**
 * Startet den Anti-Debug-Überwachungsthread.
 *
 * @return true bei erfolgreichem Start, false bei Fehler
 */
bool AntiDebug::Start() {
    // Vermeide doppelten Start
    if (isRunning) {
        return true;
    }

    // Thread starten
    isRunning = true;
    hMonitorThread = CreateThread(
        nullptr,                    // Standardsicherheitsattribute
        0,                         // Standardstackgröße
        MonitorThread,             // Thread-Funktion
        this,                      // Parameter für Thread-Funktion
        0,                         // Sofort ausführen
        nullptr                    // Thread-ID nicht benötigt
    );

    // Fehlerbehandlung
    if (!hMonitorThread) {
        isRunning = false;
        Common::Log("AntiDebug-Thread konnte nicht gestartet werden: " +
            Common::GetLastErrorString());
        return false;
    }

    Common::Log("Anti-Debug Überwachung gestartet");
    return true;
}

/**
 * Stoppt den Anti-Debug-Überwachungsthread sauber.
 */
void AntiDebug::Stop() {
    // Thread-Beendigung signalisieren und auf vollständiges Beenden warten
    isRunning = false;
    if (hMonitorThread) {
        WaitForSingleObject(hMonitorThread, INFINITE);
        CloseHandle(hMonitorThread);
        hMonitorThread = NULL;
    }
    Common::Log("Anti-Debug Überwachung gestoppt");
}

/**
 * Prüft, ob der Standarddebuggerflag in der PEB gesetzt ist.
 * Verwendet die Windows API IsDebuggerPresent().
 *
 * @return true wenn ein Debugger erkannt wurde, sonst false
 */
bool AntiDebug::IsDebuggerPresent() {
    return ::IsDebuggerPresent() != FALSE;
}

/**
 * Prüft den Debug-Port des Prozesses auf einen angehängten Debugger.
 *
 * @return true wenn ein Debugger erkannt wurde, sonst false
 */
bool AntiDebug::CheckDebugPort() {
    // ntdll.dll laden
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        return false;
    }

    // Funktion NtQueryInformationProcess dynamisch laden
    auto NtQueryInformationProcess = (NtQueryInformationProcessPtr)
        GetProcAddress(hNtdll, "NtQueryInformationProcess");

    if (!NtQueryInformationProcess) {
        return false;
    }

    // Debug-Port abfragen
    DWORD_PTR debugPort = 0;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(),
        ProcessDebugPort,
        &debugPort,
        sizeof(debugPort),
        nullptr
    );

    // Wenn NT_SUCCESS und debugPort != 0, dann ist ein Debugger angehängt
    return NT_SUCCESS(status) && debugPort != 0;
}

/**
 * Prüft die Heap-Flags, die auf einen Debugger hindeuten können.
 *
 * @return true wenn ein Debugger erkannt wurde, sonst false
 */
bool AntiDebug::CheckHeap() {
    HANDLE heap = GetProcessHeap();
    ULONG flags = 0;

    // HeapQueryInformation prüft spezielle Debug-Flags
    return HeapQueryInformation(heap, HeapCompatibilityInformation, &flags, sizeof(flags), nullptr) && flags != 0;
}

/**
 * Prüft den NtGlobalFlag in der PEB auf Debug-Indikatoren.
 *
 * @return true wenn ein Debugger erkannt wurde, sonst false
 */
bool AntiDebug::CheckNtGlobalFlag() {
    // PEB-Struktur für Anti-Debug-Zwecke definieren
    struct _PEB_LDR_DATA {
        ULONG Length;
        BOOLEAN Initialized;
        PVOID SsHandle;
        LIST_ENTRY InLoadOrderModuleList;
        LIST_ENTRY InMemoryOrderModuleList;
        LIST_ENTRY InInitializationOrderModuleList;
    };

    struct _RTL_USER_PROCESS_PARAMETERS {
        BYTE Reserved1[16];
        PVOID Reserved2[10];
        UNICODE_STRING ImagePathName;
        UNICODE_STRING CommandLine;
    };

    struct _PEB {
        BYTE Reserved1[2];
        BYTE BeingDebugged;
        BYTE Reserved2[1];
        PVOID Reserved3[2];
        struct _PEB_LDR_DATA* Ldr;
        struct _RTL_USER_PROCESS_PARAMETERS* ProcessParameters;
        PVOID Reserved4[3];
        PVOID AtlThunkSListPtr;
        PVOID Reserved5;
        ULONG Reserved6;
        PVOID Reserved7;
        ULONG Reserved8;
        ULONG AtlThunkSListPtr32;
        PVOID Reserved9[45];
        BYTE Reserved10[96];
        PVOID PostProcessInitRoutine;
        BYTE Reserved11[128];
        PVOID Reserved12[1];
        ULONG SessionId;
    };

    // Alternative Implementierung, da direkte PEB-Zugriffe instabil sein können
    BOOL debuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent);

    return debuggerPresent;
}

/**
 * Prüft, ob ein Kernel-Debugger aktiv ist.
 *
 * @return true wenn ein Kernel-Debugger erkannt wurde, sonst false
 */
bool AntiDebug::CheckKernelDebugger() {
    // ntdll.dll laden
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        return false;
    }

    // Funktion NtQuerySystemInformation dynamisch laden
    auto NtQuerySystemInformation = (NtQuerySystemInformationPtr)
        GetProcAddress(hNtdll, "NtQuerySystemInformation");

    if (!NtQuerySystemInformation) {
        return false;
    }

    // Kernel-Debugger-Status abfragen
    SYSTEM_KERNEL_DEBUGGER_INFORMATION info;
    NTSTATUS status = NtQuerySystemInformation(
        SystemKernelDebuggerInformation,
        &info,
        sizeof(info),
        nullptr
    );

    // Wenn NT_SUCCESS und KernelDebuggerEnabled, dann ist ein Kernel-Debugger aktiv
    return NT_SUCCESS(status) && info.KernelDebuggerEnabled;
}

/**
 * Versteckt den aktuellen Thread vor Debuggern durch Setzen des
 * ThreadHideFromDebugger-Flags.
 *
 * @return true bei Erfolg, false bei Fehler
 */
bool AntiDebug::HideThreadFromDebugger() {
    // ntdll.dll laden
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        return false;
    }

    // Funktion NtSetInformationThread dynamisch laden
    auto NtSetInformationThread = (NtSetInformationThreadPtr)
        GetProcAddress(hNtdll, "NtSetInformationThread");

    if (!NtSetInformationThread) {
        return false;
    }

    // ThreadHideFromDebugger-Flag setzen
    NTSTATUS status = NtSetInformationThread(
        GetCurrentThread(),
        ThreadHideFromDebugger,
        nullptr,
        0
    );

    return NT_SUCCESS(status);
}

/**
 * Führt alle Debug-Erkennungsmethoden nacheinander aus und gibt true zurück,
 * sobald eine Methode einen Debugger erkennt.
 *
 * @return true wenn ein Debugger erkannt wurde, sonst false
 */
bool AntiDebug::CheckAllDebugMethods() {
    // Standard Windows-API Debugger-Check
    if (IsDebuggerPresent()) {
        Common::Log("Standard IsDebuggerPresent erkannte einen Debugger");
        return true;
    }

    // Debug-Port-Check
    if (CheckDebugPort()) {
        Common::Log("Debug-Port erkannt");
        return true;
    }

    // Heap-Flags-Check
    if (CheckHeap()) {
        Common::Log("Heap-Flags zeigen einen Debugger an");
        return true;
    }

    // NtGlobalFlag-Check
    if (CheckNtGlobalFlag()) {
        Common::Log("NtGlobalFlag zeigt einen Debugger an");
        return true;
    }

    // Kernel-Debugger-Check
    if (CheckKernelDebugger()) {
        Common::Log("Kernel-Debugger erkannt");
        return true;
    }

    return false;  // Kein Debugger erkannt
}

/**
 * Thread-Funktion für die kontinuierliche Anti-Debug-Überwachung.
 *
 * @param lpParam Zeiger auf die AntiDebug-Instanz
 * @return Thread-Beendigungscode (0)
 */
DWORD WINAPI AntiDebug::MonitorThread(LPVOID lpParam) {
    auto antiDebug = static_cast<AntiDebug*>(lpParam);

    // Kontinuierliche Überwachung, bis isRunning auf false gesetzt wird
    while (antiDebug->isRunning) {
        if (CheckAllDebugMethods()) {
            Common::Log("Debugger erkannt!");
            // Hier weitere Aktionen einfügen, z.B. Exit oder andere Maßnahmen
        }

        // Nur einmal pro Sekunde prüfen, um CPU-Last zu reduzieren
        Sleep(1000);
    }

    return 0;
}

