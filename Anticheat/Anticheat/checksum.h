// checksum.h - Überwachung von Dateien auf Manipulation
//
// Diese Klasse überwacht wichtige Dateien auf Änderungen, indem sie
// die CRC32-Prüfsummen vergleicht, um Manipulationen zu erkennen.

#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>

class Checksum {
public:
    /**
     * Erstellt eine neue Instanz des Checksum-Monitors.
     * Initialisiert die CRC32-Tabelle.
     */
    Checksum();

    /**
     * Beendet die Überwachung und gibt alle Ressourcen frei.
     */
    ~Checksum();

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
     * Fügt eine zu überwachende Datei hinzu.
     *
     * @param path Pfad zur zu überwachenden Datei
     */
    void AddFile(const std::string& path);

    /**
     * Entfernt eine überwachte Datei.
     *
     * @param path Pfad zur zu entfernenden Datei
     */
    bool RemoveFile(const std::string& path);

    /**
     * Prüft alle überwachten Dateien auf Änderungen.
     *
     * @return true wenn keine Änderungen erkannt wurden, false bei Manipulation
     */
    bool VerifyFiles();

    /**
     * Berechnet die CRC32-Prüfsumme einer Datei.
     *
     * @param filePath Pfad zur Datei
     * @return CRC32-Prüfsumme oder 0 bei Fehler
     */
    static uint32_t CalculateCRC32(const std::string& filePath);

    /**
     * Berechnet die CRC32-Prüfsumme eines Speicherbereichs.
     *
     * @param data Zeiger auf den Speicherbereich
     * @param size Größe des Speicherbereichs in Bytes
     * @return CRC32-Prüfsumme
     */
    static uint32_t CalculateMemoryCRC32(const void* data, size_t size);

private:
    // Flag für den aktiven Zustand
    bool isRunning = false;

    // Handle zum Überwachungs-Thread
    HANDLE hMonitorThread = NULL;

    // Zuordnung von Dateipfaden zu deren Original-Prüfsummen
    std::unordered_map<std::string, uint32_t> fileChecksums;

    /**
     * Thread-Funktion für die kontinuierliche Überwachung.
     */
    static DWORD WINAPI MonitorThread(LPVOID lpParam);
};
