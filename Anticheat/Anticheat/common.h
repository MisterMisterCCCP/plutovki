// common.h - Allgemeine Hilfsfunktionen für das Anti-Cheat-System
// 
// Dieser Header definiert grundlegende Funktionen, die von verschiedenen
// Komponenten des Anti-Cheat-Systems genutzt werden, wie Logging,
// Prozessverwaltung und Zeichenkettenkonvertierung.

#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace Common {
    /**
     * Schreibt eine Nachricht mit Zeitstempel in die Log-Datei.
     *
     * @param message Die zu protokollierende Nachricht
     */
    void Log(const std::string& message);

    /**
     * Ermittelt die Fehlerbeschreibung des letzten Windows-API-Fehlers.
     *
     * @return String mit der Fehlerbeschreibung oder leer, wenn kein Fehler vorliegt
     */
    std::string GetLastErrorString();

    /**
     * Ermittelt die Prozess-ID eines laufenden Prozesses anhand seines Namens.
     *
     * @param processName Der Name des Prozesses (z.B. "notepad.exe")
     * @return Die Prozess-ID oder 0, wenn der Prozess nicht gefunden wurde
     */
    DWORD GetProcessIdByName(const std::string& processName);

    /**
     * Ermittelt alle geladenen Module (DLLs) eines Prozesses.
     *
     * @param processId Die Prozess-ID des zu untersuchenden Prozesses
     * @return Vector mit den Namen aller geladenen Module
     */
    std::vector<std::string> GetLoadedModules(DWORD processId);

    /**
     * Konvertiert einen Wide-Character-String (wchar_t*) in einen UTF-8-String.
     *
     * @param wstr Der zu konvertierende Wide-Character-String
     * @return UTF-8-kodierter std::string
     */
    std::string WideToUtf8(const wchar_t* wstr);
}
