// anti_debug.h - Schutz vor Debuggern und Analyse-Tools
//
// Diese Klasse implementiert verschiedene Techniken zur Erkennung von
// Debuggern und schützt die Anwendung vor Reverse Engineering.

#pragma once
#include <windows.h>
#include <winternl.h>

class AntiDebug {
public:
    /**
     * Erstellt eine neue Instanz des Anti-Debug-Schutzes.
     */
    AntiDebug();

    /**
     * Beendet die Überwachung und gibt alle Ressourcen frei.
     */
    ~AntiDebug();

    /**
     * Prüft, ob der Standard-Windows-Debugger angehängt ist.
     *
     * @return true wenn ein Debugger erkannt wurde, sonst false
     */
    static bool IsDebuggerPresent();

    /**
     * Prüft den Debug-Port des Prozesses auf einen angehängten Debugger.
     *
     * @return true wenn ein Debugger erkannt wurde, sonst false
     */
    static bool CheckDebugPort();

    /**
     * Prüft die Heap-Flags, die auf einen Debugger hindeuten können.
     *
     * @return true wenn ein Debugger erkannt wurde, sonst false
     */
    static bool CheckHeap();

    /**
     * Prüft den NtGlobalFlag in der PEB auf Debug-Indikatoren.
     *
     * @return true wenn ein Debugger erkannt wurde, sonst false
     */
    static bool CheckNtGlobalFlag();

    /**
     * Prüft, ob ein Kernel-Debugger aktiv ist.
     *
     * @return true wenn ein Kernel-Debugger erkannt wurde, sonst false
     */
    static bool CheckKernelDebugger();

    /**
     * Versteckt den aktuellen Thread vor Debuggern.
     *
     * @return true bei Erfolg, false bei Fehler
     */
    static bool HideThreadFromDebugger();

    /**
     * Startet den Überwachungs-Thread.
     *
     * @return true bei erfolgreicher Initialisierung, false bei Fehler
     */
    bool Start();

    /**
     * Beendet den Überwachungs-Thread.
     */
    void Stop();

private:
    // Flag für den aktiven Zustand
    bool isRunning = false;

    // Handle zum Überwachungs-Thread
    HANDLE hMonitorThread = NULL;

    /**
     * Thread-Funktion für die kontinuierliche Überwachung.
     */
    static DWORD WINAPI MonitorThread(LPVOID lpParam);

    /**
     * Führt alle Debug-Erkennungsmethoden in Kombination aus.
     *
     * @return true wenn ein Debugger mit irgendeiner Methode erkannt wurde
     */
    static bool CheckAllDebugMethods();
};
