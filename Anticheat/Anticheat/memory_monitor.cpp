// memory_monitor.cpp - Implementierung der Speicherüberwachungsfunktionen
#include "memory_monitor.h"
#include "common.h"
#include "settings.h"
#include <bitset>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

/**
 * Konstruktor: Initialisiert den MemoryMonitor.
 */
MemoryMonitor::MemoryMonitor() : isRunning(false) {
    // Keine weiteren Initialisierungen notwendig
}

/**
 * Destruktor: Stellt sicher, dass der Überwachungsthread ordnungsgemäß beendet wird.
 */
MemoryMonitor::~MemoryMonitor() {
    Stop();
}

/**
 * Startet den Überwachungsthread, falls er nicht bereits läuft.
 *
 * @return true bei erfolgreichem Start, false bei Fehler
 */
bool MemoryMonitor::Start() {
    // Vermeide doppelten Start
    if (isRunning) {
        return true;
    }

    // Thread starten
    isRunning = true;
    hMonitorThread = CreateThread(
        nullptr,                     // Standardsicherheitsattribute
        0,                          // Standardstackgröße
        MonitorThread,              // Thread-Funktion
        this,                       // Parameter für Thread-Funktion
        0,                          // Sofort ausführen
        nullptr                     // Thread-ID nicht benötigt
    );

    // Fehlerbehandlung
    if (hMonitorThread == nullptr) {
        isRunning = false;
        Common::Log("MemoryMonitor-Thread konnte nicht gestartet werden: "
            + Common::GetLastErrorString());
        return false;
    }

    Common::Log("MemoryMonitor gestartet");
    return true;
}

/**
 * Stoppt den Überwachungsthread sauber.
 */
void MemoryMonitor::Stop() {
    // Thread-Beendigung signalisieren und auf vollständiges Beenden warten
    isRunning = false;
    if (hMonitorThread) {
        WaitForSingleObject(hMonitorThread, INFINITE);
        CloseHandle(hMonitorThread);
        hMonitorThread = nullptr;
    }
    Common::Log("MemoryMonitor gestoppt");
}

/**
 * Thread-Funktion für die kontinuierliche Speicherüberwachung.
 *
 * @param lpParam Zeiger auf die MemoryMonitor-Instanz
 * @return Thread-Beendigungscode (0)
 */
DWORD WINAPI MemoryMonitor::MonitorThread(LPVOID lpParam) {
    auto monitor = static_cast<MemoryMonitor*>(lpParam);

    // Kontinuierliche Überwachung, bis isRunning auf false gesetzt wird
    while (monitor->isRunning) {
        monitor->CheckRegions();
        Sleep(GetMemoryCheckInterval());
    }

    return 0;
}

/**
 * Fügt einen Speicherbereich zur Überwachung hinzu.
 *
 * @param pid Die Prozess-ID des Zielprozesses
 * @param address Die Startadresse des zu überwachenden Speicherbereichs
 * @param size Die Größe des Speicherbereichs in Bytes
 * @return true bei erfolgreichem Hinzufügen, false bei Fehler
 */
bool MemoryMonitor::AddRegion(DWORD pid, LPVOID address, SIZE_T size) {
    // Prozesshandle öffnen mit Leserechten
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        Common::Log("Fehler beim Öffnen von Prozess " + std::to_string(pid)
            + ": " + Common::GetLastErrorString());
        return false;
    }

    // Puffer für den Speicherinhalt vorbereiten und auslesen
    std::vector<BYTE> buffer(size);
    bool success = ReadMemory(hProcess, address, size, buffer);
    CloseHandle(hProcess);

    // Fehlerbehandlung bei fehlgeschlagenem Lesen
    if (!success) {
        Common::Log("Fehler beim Lesen des Speichers an Adresse " +
            std::to_string(reinterpret_cast<uintptr_t>(address)) +
            ": " + Common::GetLastErrorString());
        return false;
    }

    // Speicherbereich zur Überwachungsliste hinzufügen
    monitoredRegions[pid].push_back({ address, size, buffer });
    Common::Log("Speicherbereich hinzugefügt: Prozess " + std::to_string(pid) +
        ", Adresse " + std::to_string(reinterpret_cast<uintptr_t>(address)) +
        ", Größe " + std::to_string(size));

    return true;
}

/**
 * Liest einen Speicherbereich aus einem fremden Prozess.
 *
 * @param hProcess Handle zum Zielprozess
 * @param address Die Startadresse des zu lesenden Speicherbereichs
 * @param size Die Größe des Speicherbereichs in Bytes
 * @param buffer Vector, der mit den gelesenen Daten gefüllt wird
 * @return true bei erfolgreichem Lesen, false bei Fehler
 */
bool MemoryMonitor::ReadMemory(HANDLE hProcess, LPVOID address, SIZE_T size, std::vector<BYTE>& buffer) {
    buffer.resize(size);
    SIZE_T bytesRead;

    // Windows-API zum Lesen fremder Prozessspeicherbereiche
    return ReadProcessMemory(hProcess, address, buffer.data(), size, &bytesRead) && bytesRead == size;
}

/**
 * Entfernt einen überwachten Speicherbereich.
 *
 * @param pid Die Prozess-ID des Zielprozesses
 * @param address Die Startadresse des zu entfernenden Speicherbereichs
 * @return true wenn erfolgreich entfernt, false wenn nicht gefunden
 */
bool MemoryMonitor::RemoveRegion(DWORD pid, LPVOID address) {
    // Prozess in der Überwachungsliste suchen
    auto it = monitoredRegions.find(pid);
    if (it != monitoredRegions.end()) {
        auto& regions = it->second;
        size_t originalSize = regions.size();

        // Speicherbereich mit der angegebenen Adresse entfernen
        regions.erase(
            std::remove_if(regions.begin(), regions.end(),
                [address](const MemoryRegion& r) { return r.address == address; }),
            regions.end());

        // Prüfen, ob tatsächlich etwas entfernt wurde
        if (regions.size() < originalSize) {
            Common::Log("Speicherbereich entfernt: Prozess " + std::to_string(pid) +
                ", Adresse " + std::to_string(reinterpret_cast<uintptr_t>(address)));
            return true;
        }
    }

    return false;  // Nicht gefunden oder nicht entfernt
}

/**
 * Prüft alle überwachten Speicherbereiche auf Änderungen.
 *
 * @return true wenn die Prüfung erfolgreich durchgeführt wurde
 */
bool MemoryMonitor::CheckRegions() {
    // Jeden überwachten Prozess durchgehen
    for (auto& processEntry : monitoredRegions) {
        DWORD pid = processEntry.first;
        auto& regions = processEntry.second;

        // Prozesshandle öffnen mit Leserechten
        HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) {
            Common::Log("Prozess " + std::to_string(pid) + " kann nicht geöffnet werden: " +
                Common::GetLastErrorString());
            continue;  // Mit nächstem Prozess fortfahren
        }

        // Jeden überwachten Speicherbereich des Prozesses prüfen
        for (auto& region : regions) {
            std::vector<BYTE> currentData(region.size);

            // Aktuellen Speicherinhalt auslesen
            if (ReadMemory(hProcess, region.address, region.size, currentData)) {
                // Vergleichen mit dem ursprünglichen Inhalt
                if (currentData != region.originalData) {
                    Common::Log("Speichermanipulation erkannt bei Adresse: "
                        + std::to_string(reinterpret_cast<uintptr_t>(region.address)));
                    // Hier könnten weitere Aktionen erfolgen, z.B. Prozess beenden
                }
            }
            else {
                Common::Log("Konnte Speicher nicht lesen an Adresse: " +
                    std::to_string(reinterpret_cast<uintptr_t>(region.address)));
            }
        }

        CloseHandle(hProcess);
    }

    return true;
}

