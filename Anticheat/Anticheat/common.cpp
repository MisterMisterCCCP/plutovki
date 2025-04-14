// common.cpp - Implementierung allgemeiner Hilfsfunktionen für das Anti-Cheat-System
#include "common.h"
#include <iostream>
#include <fstream>
#include <tlhelp32.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <string.h>
#include "settings.h"

/**
 * Konvertiert einen Wide-Character-String (wchar_t) in einen UTF-8-String.
 * Diese Funktion ist wichtig für die Interaktion mit Windows-API-Funktionen,
 * die häufig mit Wide-Strings arbeiten.
 *
 * @param wstr Der zu konvertierende Wide-Character-String
 * @return UTF-8-kodierter std::string oder leerer String bei NULL-Eingabe
 */
std::string Common::WideToUtf8(const wchar_t* wstr) {
    // Null-Pointer-Prüfung
    if (!wstr) {
        return "";
    }

    // Erst die benötigte Größe ermitteln (inklusive Null-Terminator)
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size <= 1) {
        return "";  // Fehlerhaftes oder leeres Ergebnis
    }

    // String mit richtiger Größe vorbelegen (ohne Null-Terminator)
    std::string result(size - 1, 0);

    // Tatsächliche Konvertierung durchführen
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, NULL, NULL);
    return result;
}

/**
 * Schreibt eine Nachricht mit aktuellem Zeitstempel in die Log-Datei.
 * Diese Funktion dient zur zentralen Protokollierung von Ereignissen
 * im Anti-Cheat-System.
 *
 * @param message Die zu protokollierende Nachricht
 */
void Common::Log(const std::string& message) {
    // Aktuellen Zeitstempel erzeugen
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);

    // Formatierte Nachricht mit Zeitstempel erstellen
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "[%H:%M:%S] ") << message;

    // Logdatei-Pfad aus der Konfiguration lesen
    std::string logFilePath = GetLogFilePath();

    // In Logdatei schreiben (anhängen)
    std::ofstream logFile(logFilePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << ss.str() << std::endl;
    }
    else {
        // Konnte Logdatei nicht öffnen - Ausgabe auf Konsole
        std::cerr << "Fehler beim Öffnen der Logdatei: " << logFilePath << std::endl;
        std::cerr << ss.str() << std::endl;
    }
}

/**
 * Liefert eine lesbare Fehlerbeschreibung zum letzten Windows-API-Fehler.
 * Diese Funktion ist nützlich, um detaillierte Fehlerinformationen für
 * die Protokollierung zu erhalten.
 *
 * @return Fehlerbeschreibung als String oder leerer String, wenn kein Fehler vorliegt
 */
std::string Common::GetLastErrorString() {
    // Letzten Fehlercode abfragen
    DWORD error = GetLastError();
    if (error == 0) {
        return "";  // Kein Fehler
    }

    // Fehlermeldung von Windows abrufen
    LPSTR buffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, error, 0, (LPSTR)&buffer, 0, nullptr);

    // Meldung aus dem Buffer kopieren
    std::string message(buffer, size);

    // Von Windows allokierten Speicher freigeben
    LocalFree(buffer);
    return message;
}

/**
 * Ermittelt die Prozess-ID eines laufenden Prozesses anhand seines Namens.
 * Diese Funktion durchsucht alle laufenden Prozesse und gibt die ID des
 * ersten Prozesses zurück, dessen Name dem gesuchten entspricht.
 *
 * @param processName Der Name des gesuchten Prozesses (z.B. "explorer.exe")
 * @return Die Prozess-ID oder 0, wenn der Prozess nicht gefunden wurde
 */
DWORD Common::GetProcessIdByName(const std::string& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    // Snapshot aller laufenden Prozesse erstellen
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        Common::Log("Fehler beim Erstellen des Prozess-Snapshots: " + GetLastErrorString());
        return 0;
    }

    // Prozessliste durchsuchen
    if (Process32First(snapshot, &entry)) {
        do {
            // WCHAR zu char konvertieren für Stringvergleich
            char exeFile[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, entry.szExeFile, -1,
                exeFile, MAX_PATH, NULL, NULL);

            // Fall-insensitiver Vergleich der Prozessnamen
            if (_stricmp(exeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    // Aufräumen und bei Nicht-Erfolg 0 zurückgeben
    CloseHandle(snapshot);
    return 0;
}

/**
 * Ermittelt alle in einem Prozess geladenen Module (DLLs).
 * Diese Funktion ist nützlich, um nachträglich geladene DLLs zu erkennen,
 * die auf Cheat-Injektionen hindeuten könnten.
 *
 * @param processId Die ID des zu untersuchenden Prozesses
 * @return Ein Vector mit den Namen aller geladenen Module oder ein leerer
 *         Vector, wenn der Prozess nicht geöffnet werden konnte
 */
std::vector<std::string> Common::GetLoadedModules(DWORD processId) {
    std::vector<std::string> modules;
    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);

    // Snapshot aller Module im Prozess erstellen
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        Common::Log("Fehler beim Erstellen des Modul-Snapshots für Prozess " +
            std::to_string(processId) + ": " + GetLastErrorString());
        return modules;  // Leerer Vector bei Fehler
    }

    // Alle Module abrufen
    if (Module32First(snapshot, &entry)) {
        do {
            // WCHAR zu char konvertieren für std::string
            char moduleName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, entry.szModule, -1,
                moduleName, MAX_PATH, NULL, NULL);
            modules.push_back(moduleName);
        } while (Module32Next(snapshot, &entry));
    }

    // Aufräumen und Ergebnis zurückgeben
    CloseHandle(snapshot);
    return modules;
}

