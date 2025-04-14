// process_monitor.h - Überwachung von Prozessen zur Cheat-Erkennung
//
// Diese Klasse überwacht laufende Prozesse auf dem System und meldet
// verdächtige Prozesse sowie nachträglich geladene Module, die auf
// Injection-Versuche hindeuten könnten.

#pragma once
#include <windows.h>
#include <string>
#include <vector>

class ProcessMonitor {
public:
    /**
     * Erstellt eine neue Instanz des Prozess-Monitors.
     */
    ProcessMonitor();

    /**
     * Beendet die Überwachung und gibt alle Ressourcen frei.
     */
    ~ProcessMonitor();

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

    /**
     * Legt den zu überwachenden Prozess fest und speichert dessen initiale Module.
     *
     * @param pid Die Prozess-ID des zu überwachenden Prozesses
     */
    void SetTargetProcess(DWORD pid);

    /**
     * Prüft, ob verdächtige Prozesse (Cheat-Tools) aktiv sind.
     *
     * @return true wenn erfolgreich geprüft, false bei Fehler
     */
    bool CheckSuspiciousProcesses();

    /**
     * Prüft, ob neue Module (DLLs) in den Zielprozess injiziert wurden.
     *
     * @return true wenn erfolgreich geprüft, false bei Fehler
     */
    bool CheckForInjection();

private:
    // Flag für den aktiven Zustand
    bool isRunning = false;

    // Prozess-ID des zu überwachenden Prozesses
    DWORD targetProcessId = 0;

    // Handle zum Überwachungs-Thread
    HANDLE hMonitorThread = nullptr;

    // Modul-Liste zum Zeitpunkt der Initialisierung
    std::vector<std::string> initialModules;

    /**
     * Thread-Funktion für die kontinuierliche Überwachung.
     */
    static DWORD WINAPI MonitorThread(LPVOID lpParam);

    /**
     * Listet alle aktiven Prozesse auf.
     *
     * @param output Vector, der mit Prozess-IDs gefüllt wird
     * @return true bei erfolgreicher Auflistung, false bei Fehler
     */
    bool EnumerateProcesses(std::vector<DWORD>& output);

    /**
     * Ermittelt den Namen eines Prozesses anhand seiner ID.
     *
     * @param pid Die Prozess-ID
     * @param name Rückgabe-String für den Prozessnamen
     * @return true bei erfolgreicher Ermittlung, false bei Fehler
     */
    bool GetProcessName(DWORD pid, std::string& name);
};
