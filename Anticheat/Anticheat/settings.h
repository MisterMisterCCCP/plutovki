// settings.h - Konfiguration für das Anti-Cheat-System
//
// Diese Datei enthält globale Einstellungen und Konstanten, die von
// verschiedenen Komponenten des Anti-Cheat-Systems verwendet werden.

#pragma once
#include <string>
#include <vector>
#include "config.h"

    // Die Standard-Konfigurationen können aus der Konfigurationsdatei überschrieben werden

    // Dateipfade für zu überwachende Dateien
    inline std::string GetMainExePath() {
        return Config::GetString("Files", "MainExePath", "main.exe");
    }

    inline std::string GetSettingsPath() {
        return Config::GetString("Files", "SettingsPath", "settings.ini");
    }

    inline std::string GetLogFilePath() {
        return Config::GetString("Files", "LogFilePath", "anticheat.log");
    }

    // Liste von Prozessnamen, die als potenzielle Cheat-Tools erkannt werden sollen.
    inline std::vector<std::string> GetSuspiciousProcesses() {
        std::vector<std::string> defaultProcesses = {
            "cheatengine.exe", "CheatEngine.exe", "ollydbg.exe",
            "x64dbg.exe", "x32dbg.exe", "ida.exe", "ida64.exe"
        };

        return Config::GetList("Detection", "SuspiciousProcesses");
    }

    // Liste von DLL-Namen, die als verdächtig gelten
    inline std::vector<std::string> GetSuspiciousDLLs() {
        std::vector<std::string> defaultDLLs = {
            "speedhack.dll", "inject.dll", "hook.dll"
        };

        return Config::GetList("Detection", "SuspiciousDLLs");
    }

    // Zeitintervalle in Millisekunden für verschiedene Überprüfungen
    inline int GetProcessCheckInterval() {
        return Config::GetInt("Intervals", "ProcessCheckInterval", 1000);
    }

    inline int GetMemoryCheckInterval() {
        return Config::GetInt("Intervals", "MemoryCheckInterval", 500);
    }

    inline int GetChecksumCheckInterval() {
        return Config::GetInt("Intervals", "ChecksumCheckInterval", 2000);
    }

    // Erstellt eine Standardkonfigurationsdatei, falls noch keine existiert
    inline void CreateDefaultConfigIfNeeded(const std::string& configPath = "config.ini") {
        // Prüfen, ob die Datei bereits existiert
        DWORD fileAttributes = GetFileAttributesA(configPath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            return; // Datei existiert bereits
        }

        // Standardwerte setzen
        Config::SetString("Files", "MainExePath", "main.exe");
        Config::SetString("Files", "SettingsPath", "settings.ini");
        Config::SetString("Files", "LogFilePath", "anticheat.log");

        // Suspicious Processes
        std::vector<std::string> suspiciousProcesses = {
            "cheatengine.exe", "CheatEngine.exe", "ollydbg.exe",
            "x64dbg.exe", "x32dbg.exe", "ida.exe", "ida64.exe"
        };
        Config::SetList("Detection", "SuspiciousProcesses", suspiciousProcesses);

        // Suspicious DLLs
        std::vector<std::string> suspiciousDLLs = {
            "speedhack.dll", "inject.dll", "hook.dll"
        };
        Config::SetList("Detection", "SuspiciousDLLs", suspiciousDLLs);

        // Zeitintervalle
        Config::SetInt("Intervals", "ProcessCheckInterval", 1000);
        Config::SetInt("Intervals", "MemoryCheckInterval", 500);
        Config::SetInt("Intervals", "ChecksumCheckInterval", 2000);

        // Speichern der Konfigurationsdatei
        Config::Save(configPath);
    }

