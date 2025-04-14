// checksum.cpp - Implementierung der Dateintegritätsprüfung
#include "checksum.h"
#include "common.h"
#include <fstream>
#include <bitset>
#include <Windows.h>
#include "settings.h"

// CRC32-Lookup-Tabelle für schnelle Berechnung
static uint32_t crc_table[256];

/**
 * Generiert die CRC32-Lookup-Tabelle für effizientere Berechnungen.
 * Basiert auf dem Standard-CRC32-Polynom 0xEDB88320.
 */
static void GenerateCRCTable() {
    const uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++) {
            if (c & 1) {
                c = polynomial ^ (c >> 1);
            }
            else {
                c >>= 1;
            }
        }
        crc_table[i] = c;
    }
}

/**
 * Konstruktor: Initialisiert die CRC32-Tabelle.
 */
Checksum::Checksum() {
    GenerateCRCTable();
}

/**
 * Destruktor: Stellt sicher, dass der Überwachungsthread beendet wird.
 */
Checksum::~Checksum() {
    Stop();
}

/**
 * Berechnet die CRC32-Prüfsumme einer Datei.
 *
 * @param filePath Pfad zur Datei
 * @return CRC32-Prüfsumme oder 0 bei Fehler
 */
uint32_t Checksum::CalculateCRC32(const std::string& filePath) {
    // Datei im Binärmodus öffnen
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        Common::Log("Datei konnte nicht geöffnet werden: " + filePath);
        return 0;
    }

    // Initialisierung der CRC-Berechnung
    uint32_t crc = 0xFFFFFFFF;
    char buffer[4096];  // 4KB Puffer für effizientes Lesen

    // Hauptleseschleife
    while (file.read(buffer, sizeof(buffer))) {
        for (size_t i = 0; i < file.gcount(); i++) {
            crc = crc_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
        }
    }

    // Verarbeite verbleibende Bytes beim letzten Lesevorgang
    if (file.gcount() > 0) {
        for (size_t i = 0; i < file.gcount(); i++) {
            crc = crc_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
        }
    }

    // Finalisierung der CRC durch XOR mit 0xFFFFFFFF
    return crc ^ 0xFFFFFFFF;
}

/**
 * Prüft alle überwachten Dateien auf Veränderungen durch Vergleich der CRC32-Prüfsummen.
 *
 * @return true wenn keine Veränderungen festgestellt wurden, false bei Manipulation
 */
bool Checksum::VerifyFiles() {
    for (const auto& pair : fileChecksums) {
        const std::string& path = pair.first;
        const uint32_t original = pair.second;

        // Aktuelle Prüfsumme berechnen und mit der gespeicherten vergleichen
        uint32_t current = CalculateCRC32(path);
        if (current != original) {
            Common::Log("Dateimanipulation erkannt: " + path);
            return false;
        }
    }
    return true;
}

/**
 * Berechnet die CRC32-Prüfsumme eines Speicherbereichs.
 *
 * @param data Zeiger auf den Speicherbereich
 * @param size Größe des Speicherbereichs in Bytes
 * @return CRC32-Prüfsumme
 */
uint32_t Checksum::CalculateMemoryCRC32(const void* data, size_t size) {
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);

    // Byte für Byte durch den Speicher iterieren
    for (size_t i = 0; i < size; i++) {
        crc = crc_table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

/**
 * Fügt eine Datei zur Überwachungsliste hinzu.
 *
 * @param path Pfad zur zu überwachenden Datei
 */
void Checksum::AddFile(const std::string& path) {
    // Prüfen, ob die Datei existiert
    DWORD fileAttributes = GetFileAttributesA(path.c_str());
    bool fileExists = (fileAttributes != INVALID_FILE_ATTRIBUTES &&
        !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));

    if (fileExists) {
        // Initial-Prüfsumme berechnen und speichern
        uint32_t checksum = CalculateCRC32(path);
        fileChecksums[path] = checksum;
        Common::Log("Datei zum Checksummen-Monitor hinzugefügt: " + path);
    }
    else {
        Common::Log("Datei existiert nicht: " + path);
    }
}

/**
 * Entfernt eine Datei aus der Überwachungsliste.
 *
 * @param path Pfad zur zu entfernenden Datei
 */
bool Checksum::RemoveFile(const std::string& path) {
    auto it = fileChecksums.find(path);
    if (it != fileChecksums.end()) {
        fileChecksums.erase(it);
        Common::Log("Datei aus Checksummen-Monitor entfernt: " + path);
        return true;
	}return false;
}

/**
 * Startet den Überwachungsthread, falls er nicht bereits läuft.
 *
 * @return true bei erfolgreichem Start, false bei Fehler
 */
bool Checksum::Start() {
    // Vermeide doppelten Start
    if (isRunning) {
        return true;
    }

    // Thread starten
    isRunning = true;
    hMonitorThread = CreateThread(
        NULL,                        // Standardsicherheitsattribute
        0,                          // Standardstackgröße
        MonitorThread,              // Thread-Funktion
        this,                       // Parameter für Thread-Funktion
        0,                          // Sofort ausführen
        NULL                        // Thread-ID nicht benötigt
    );

    // Fehlerbehandlung
    if (hMonitorThread == NULL) {
        Common::Log("Fehler beim Starten des Checksummen-Monitor-Threads");
        isRunning = false;
        return false;
    }

    Common::Log("Checksummen-Monitor gestartet");
    return true;
}

/**
 * Stoppt den Überwachungsthread sauber.
 */
void Checksum::Stop() {
    // Nur wenn Thread läuft
    if (!isRunning) {
        return;
    }

    // Thread-Beendigung signalisieren und auf vollständiges Beenden warten
    isRunning = false;

    if (hMonitorThread != NULL) {
        WaitForSingleObject(hMonitorThread, INFINITE);
        CloseHandle(hMonitorThread);
        hMonitorThread = NULL;
    }

    Common::Log("Checksummen-Monitor gestoppt");
}

/**
 * Thread-Funktion für die kontinuierliche Überwachung der Dateien.
 *
 * @param lpParam Zeiger auf die Checksum-Instanz
 * @return Thread-Beendigungscode (0)
 */
DWORD WINAPI Checksum::MonitorThread(LPVOID lpParam) {
    Checksum* instance = static_cast<Checksum*>(lpParam);

    // Kontinuierliche Überwachung, bis isRunning auf false gesetzt wird
    while (instance->isRunning) {
        instance->VerifyFiles();
        Sleep(GetChecksumCheckInterval());
    }

    return 0;
}

