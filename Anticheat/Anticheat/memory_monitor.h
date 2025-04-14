// memory_monitor.h - Überwachung von Speicherbereichen zur Cheat-Erkennung
//
// Diese Klasse überwacht bestimmte Speicherbereiche in Prozessen und
// erkennt unautorisierte Änderungen, die auf Cheats hindeuten könnten.

#pragma once
#include <windows.h>
#include <vector>
#include <unordered_map>

/**
 * Repräsentiert einen zu überwachenden Speicherbereich.
 */
struct MemoryRegion {
    // Adresse des zu überwachenden Speicherbereichs
    LPVOID address;

    // Größe des Speicherbereichs in Bytes
    SIZE_T size;

    // Originaler Inhalt des Speicherbereichs als Referenz
    std::vector<BYTE> originalData;
};

class MemoryMonitor {
public:
    /**
     * Erstellt eine neue Instanz des Speicher-Monitors.
     */
    MemoryMonitor();

    /**
     * Beendet die Überwachung und gibt alle Ressourcen frei.
     */
    ~MemoryMonitor();

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
     * Fügt einen zu überwachenden Speicherbereich hinzu.
     *
     * @param pid Die Prozess-ID des Zielprozesses
     * @param address Die Startadresse des Speicherbereichs
     * @param size Die Größe des Speicherbereichs in Bytes
     * @return true bei erfolgreicher Hinzufügung, false bei Fehler
     */
    bool AddRegion(DWORD pid, LPVOID address, SIZE_T size);

    /**
     * Entfernt einen überwachten Speicherbereich.
     *
     * @param pid Die Prozess-ID des Zielprozesses
     * @param address Die Startadresse des zu entfernenden Speicherbereichs
     * @return true wenn erfolgreich entfernt, false wenn nicht gefunden
     */
    bool RemoveRegion(DWORD pid, LPVOID address);

    /**
     * Prüft alle überwachten Speicherbereiche auf Änderungen.
     *
     * @return true wenn erfolgreich geprüft, false bei Fehler
     */
    bool CheckRegions();

private:
    // Flag für den aktiven Zustand
    bool isRunning = false;

    // Handle zum Überwachungs-Thread
    HANDLE hMonitorThread = NULL;

    // Zuordnung von Prozess-IDs zu überwachten Speicherbereichen
    std::unordered_map<DWORD, std::vector<MemoryRegion>> monitoredRegions;

    /**
     * Thread-Funktion für die kontinuierliche Überwachung.
     */
    static DWORD WINAPI MonitorThread(LPVOID lpParam);

    /**
     * Liest einen Speicherbereich aus einem fremden Prozess.
     *
     * @param hProcess Handle zum Zielprozess
     * @param address Die Startadresse des zu lesenden Speicherbereichs
     * @param size Die Größe des Speicherbereichs in Bytes
     * @param buffer Vector, der mit den gelesenen Daten gefüllt wird
     * @return true bei erfolgreichem Lesen, false bei Fehler
     */
    bool ReadMemory(HANDLE hProcess, LPVOID address, SIZE_T size, std::vector<BYTE>& buffer);
};
