// process_monitor.cpp - Implementierung der Prozessüberwachungsfunktionen
#include "process_monitor.h"
#include "common.h"
#include "settings.h"
#include <tlhelp32.h>
#include <cwchar>
#include <algorithm>

/**
 * Konstruktor: Initialisiert den ProcessMonitor mit Standardwerten.
 */
ProcessMonitor::ProcessMonitor() : isRunning(false), targetProcessId(0) {
    // Keine weiteren Initialisierungen notwendig
}

/**
 * Destruktor: Stellt sicher, dass der Überwachungsthread ordnungsgemäß gestoppt wird.
 */
ProcessMonitor::~ProcessMonitor() {
    Stop();
}

/**
 * Startet den Überwachungsthread, falls er nicht bereits läuft.
 *
 * @return true bei erfolgreichem Start, false bei Fehler
 */
bool ProcessMonitor::Start() {
    // Vermeide doppelten Start
    if (isRunning) {
        return true;
    }

    // Thread starten
    isRunning = true;
    hMonitorThread = CreateThread(
        nullptr,                       // Standardsicherheitsattribute
        0,                            // Standardstackgröße
        &ProcessMonitor::MonitorThread, // Thread-Funktion
        this,                         // Parameter für Thread-Funktion
        0,                            // Sofort ausführen
        nullptr                       // Thread-ID nicht benötigt
    );

    // Fehlerbehandlung
    if (!hMonitorThread) {
        isRunning = false;
        Common::Log("ProcessMonitor-Thread konnte nicht gestartet werden: "
            + Common::GetLastErrorString());
        return false;
    }

    return true;
}

/**
 * Legt den zu überwachenden Prozess fest und speichert initial geladene Module.
 *
 * @param pid Die Prozess-ID des zu überwachenden Prozesses
 */
void ProcessMonitor::SetTargetProcess(DWORD pid) {
    targetProcessId = pid;
    // Initiale Liste der Module speichern, um später Änderungen zu erkennen
    initialModules = Common::GetLoadedModules(targetProcessId);
}

/**
 * Stoppt den Überwachungsthread sauber.
 */
void ProcessMonitor::Stop() {
    // Thread-Beendigung signalisieren und auf vollständiges Beenden warten
    isRunning = false;
    if (hMonitorThread) {
        WaitForSingleObject(hMonitorThread, INFINITE);
        CloseHandle(hMonitorThread);
        hMonitorThread = nullptr;
    }
}

/**
 * Thread-Funktion für die kontinuierliche Prozessüberwachung.
 *
 * @param lpParam Zeiger auf die ProcessMonitor-Instanz
 * @return Thread-Beendigungscode (0)
 */
DWORD WINAPI ProcessMonitor::MonitorThread(LPVOID lpParam) {
    ProcessMonitor* monitor = static_cast<ProcessMonitor*>(lpParam);

    // Kontinuierliche Überwachung, bis isRunning auf false gesetzt wird
    while (monitor->isRunning) {
        // Prüfe auf bekannte Cheat-Prozesse im System
        monitor->CheckSuspiciousProcesses();

        // Nur wenn ein Zielprozess gesetzt ist, auf DLL-Injection prüfen
        if (monitor->targetProcessId != 0) {
            monitor->CheckForInjection();
        }

        // Warte bis zum nächsten Prüfzyklus
        Sleep(GetProcessCheckInterval());
    }

    return 0;
}

/**
 * Durchsucht das System nach bekannten Cheat-Programmen.
 *
 * @return true wenn die Prüfung erfolgreich durchgeführt wurde, false bei Fehler
 */
bool ProcessMonitor::CheckSuspiciousProcesses() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    // Snapshot aller laufenden Prozesse erstellen
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        Common::Log("Fehler beim Erstellen des Prozess-Snapshots: " + Common::GetLastErrorString());
        return false;
    }

    // Prozessliste durchsuchen
    if (Process32FirstW(snapshot, &entry)) {
        do {
            // Jeden Prozess mit der Liste verdächtiger Prozesse vergleichen
            for (const auto& name : GetSuspiciousProcesses() ){
                // Konvertiere std::string zu std::wstring für den Vergleich
                std::wstring wname;
                wname.reserve(name.length());
                for (size_t i = 0; i < name.length(); i++) {
                    wname.push_back(static_cast<wchar_t>(name[i]));
                }

                // Case-insensitiver Vergleich
                if (_wcsicmp(entry.szExeFile, wname.c_str()) == 0) {
                    Common::Log("Verdächtiger Prozess gefunden: " + name);
                    // Hier könnten weitere Aktionen erfolgen, z.B. Alarm auslösen
                }
            }
        
    } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return true;
}

/**
 * Prüft, ob neue DLLs in den Zielprozess geladen wurden, die auf eine Injection hindeuten.
 *
 * @return true wenn die Prüfung erfolgreich durchgeführt wurde, false bei Fehler
 */
bool ProcessMonitor::CheckForInjection() {
    // Sicherheitscheck: Muss ein Zielprozess gesetzt sein
    if (targetProcessId == 0) {
        return false;
    }

    // Aktuelle Liste der geladenen Module abrufen
    auto currentModules = Common::GetLoadedModules(targetProcessId);

    // Bei erster Ausführung: Speichere aktuelle Module als Referenz
    if (initialModules.empty()) {
        initialModules = currentModules;
        return true;
    }

    // Prüfe, ob neue Module hinzugefügt wurden
    for (const auto& module : currentModules) {
        bool found = false;
        // Suche nach dem Modul in der initialen Liste
        for (const auto& initialModule : initialModules) {
            if (module == initialModule) {
                found = true;
                break;
            }
        }

        // Neues Modul gefunden - mögliche DLL-Injection!
        if (!found) {
            Common::Log("Verdächtige DLL injiziert: " + module);
            // Hier weitere Aktionen einfügen, z.B. Prozess beenden
        }
    }

    return true;
}
