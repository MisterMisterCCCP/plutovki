// main.cpp - Haupteinstiegspunkt und Steuerung des Anti-Cheat-Systems
#include "common.h"
#include "process_monitor.h"
#include "memory_monitor.h"
#include "checksum.h"
#include "anti_debug.h"
#include "settings.h"
#include "config.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>

/**
 * Überprüft, ob das Programm mit Administratorrechten läuft.
 *
 * @return true wenn Administratorrechte vorhanden sind, sonst false
 */
bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    // SID für die Administratorgruppe erzeugen
    if (!AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup))
    {
        return false;
    }

    // Überprüfen, ob der aktuelle Benutzer Mitglied ist
    if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
        isAdmin = FALSE;
    }

    FreeSid(adminGroup);
    return isAdmin != FALSE;
}

/**
 * Zeigt die verfügbaren Befehle und deren Beschreibungen an.
 */
void ShowHelp() {
    std::cout << "\nVerfügbare Befehle:\n";
    std::cout << "  stop               - Beendet das Anti-Cheat-System\n";
    std::cout << "  status             - Zeigt den Status aller Komponenten an\n";
    std::cout << "  help               - Zeigt diese Hilfe an\n";
    std::cout << "  config             - Zeigt die aktuelle Konfiguration an\n";
    std::cout << "  addfile <pfad>     - Fügt eine Datei zur Integritätsprüfung hinzu\n";
    std::cout << "  removefile <pfad>  - Entfernt eine Datei von der Integritätsprüfung\n";
    std::cout << "  checkfiles         - Führt sofort eine Integritätsprüfung durch\n";
    std::cout << "  checkprocess <name>- Prüft, ob ein bestimmter Prozess läuft\n";
}

/**
 * Zeigt den Status aller Komponenten an.
 *
 * @param procMonitor Referenz auf den ProcessMonitor
 * @param memMonitor Referenz auf den MemoryMonitor
 * @param checksum Referenz auf den Checksum-Monitor
 * @param antiDebug Referenz auf den AntiDebug-Monitor
 */
void ShowStatus(const ProcessMonitor& procMonitor,
    const MemoryMonitor& memMonitor,
    const Checksum& checksum,
    const AntiDebug& antiDebug)
{
    std::cout << "\n=== Anti-Cheat-System Status ===\n";

    // Administrative Rechte
    std::cout << "Administrative Rechte: "
        << (IsRunningAsAdmin() ? "Ja" : "Nein") << "\n";

    // Überwachter Prozess
    std::cout << "Überwachter Prozess: " << GetCurrentProcessId() << " (selbst)\n";

    // Zeitintervalle
    std::cout << "Prozessüberwachung-Intervall: "
        << GetProcessCheckInterval() << " ms\n";
    std::cout << "Speicherüberwachung-Intervall: "
        << GetMemoryCheckInterval() << " ms\n";
    std::cout << "Checksummen-Intervall: "
        << GetChecksumCheckInterval() << " ms\n";

    // Überwachte Dateien - nicht direkt verfügbar, man könnte eine Funktion in Checksum hinzufügen
    std::cout << "Überwachte Dateien: [Nicht verfügbar in dieser Version]\n";

    // Verdächtige Prozesse
    std::cout << "Verdächtige Prozesse (Anzahl): "
        << GetSuspiciousProcesses().size() << "\n";

    // Laufzeit
    static auto startTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto runTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

    std::cout << "Laufzeit: " << runTime << " Sekunden\n";
}

/**
 * Zeigt die aktuelle Konfiguration an
 */
void ShowConfiguration() {
    std::cout << "\n=== Anti-Cheat-System Konfiguration ===\n";

    // Pfade
    std::cout << "Hauptprogramm-Pfad: " << GetMainExePath() << "\n";
    std::cout << "Einstellungsdatei-Pfad: " << GetSettingsPath() << "\n";
    std::cout << "Log-Datei-Pfad: " << GetLogFilePath() << "\n\n";

    // Verdächtige Prozesse
    std::cout << "Verdächtige Prozesse:\n";
    for (const auto& proc : GetSuspiciousProcesses()) {
        std::cout << "  - " << proc << "\n";
    }
    std::cout << "\n";

    // Verdächtige DLLs
    std::cout << "Verdächtige DLLs:\n";
    for (const auto& dll : GetSuspiciousDLLs()) {
        std::cout << "  - " << dll << "\n";
    }
    std::cout << "\n";

    // Zeitintervalle
    std::cout << "Prozessüberwachung-Intervall: " << GetProcessCheckInterval() << " ms\n";
    std::cout << "Speicherüberwachung-Intervall: " << GetMemoryCheckInterval() << " ms\n";
    std::cout << "Checksummen-Intervall: " << GetChecksumCheckInterval() << " ms\n";
}

/**
 * Hauptfunktion des Anti-Cheat-Systems.
 * Initialisiert alle Komponenten, startet die Überwachung und
 * bietet eine erweiterte Kommandozeilenschnittstelle.
 */
int main() {
    //------------------------------------------------------------------------
    // 1. Initialisierung
    //------------------------------------------------------------------------

    // Konfiguration überprüfen und ggf. erstellen
    CreateDefaultConfigIfNeeded("config.ini");

    // Administratorrechte überprüfen
    if (!IsRunningAsAdmin()) {
        std::cerr << "WARNUNG: Dieses Programm läuft nicht mit Administratorrechten.\n";
        std::cerr << "Einige Funktionen könnten eingeschränkt sein.\n\n";
    }

    // Begrüßungsmeldung
    std::cout << "=== Anti-Cheat System wird initialisiert ===\n";
    Common::Log("Anti-Cheat System gestartet");

    // Debugger-Schutz aktivieren
    if (AntiDebug::HideThreadFromDebugger()) {
        Common::Log("Thread erfolgreich vor Debuggern versteckt");
    }
    else {
        Common::Log("Konnte Thread nicht vor Debuggern verstecken");
    }

    //------------------------------------------------------------------------
    // 2. Komponenten erstellen
    //------------------------------------------------------------------------

    // Erstelle Instanzen aller Schutzkomponenten
    ProcessMonitor procMonitor;
    MemoryMonitor memMonitor;
    Checksum checksum;
    AntiDebug antiDebug;

    // Eigenen Prozess überwachen
    procMonitor.SetTargetProcess(GetCurrentProcessId());
    Common::Log("Überwachung für aktuellen Prozess eingerichtet");

    // Kritische Dateien zur Überwachung hinzufügen
    checksum.AddFile(GetMainExePath());
    checksum.AddFile(GetSettingsPath());

    DWORD configAttrs = GetFileAttributesA("config.ini");
    if (configAttrs != INVALID_FILE_ATTRIBUTES && !(configAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
        checksum.AddFile("config.ini");
    }

    //------------------------------------------------------------------------
    // 3. Überwachung starten
    //------------------------------------------------------------------------

    bool allStarted = true;

    // Prozessmonitor starten
    if (!procMonitor.Start()) {
        std::cerr << "Fehler beim Starten des Prozess-Monitors\n";
        allStarted = false;
    }

    // Speichermonitor starten
    if (!memMonitor.Start()) {
        std::cerr << "Fehler beim Starten des Speicher-Monitors\n";
        allStarted = false;
    }

    // Checksum-Monitor starten
    if (!checksum.Start()) {
        std::cerr << "Fehler beim Starten der Checksum-Prüfung\n";
        allStarted = false;
    }

    // Anti-Debug-Monitor starten
    if (!antiDebug.Start()) {
        std::cerr << "Fehler beim Starten des Anti-Debug-Schutzes\n";
        allStarted = false;
    }

    // Status ausgeben
    if (allStarted) {
        std::cout << "Anti-Cheat-System erfolgreich aktiviert\n";
    }
    else {
        std::cout << "Anti-Cheat-System mit Fehlern aktiviert\n";
    }

    //------------------------------------------------------------------------
    // 4. Erweiterte Kommandozeilenschnittstelle
    //------------------------------------------------------------------------

    std::cout << "\nAnti-Cheat-System bereit. Geben Sie 'help' für verfügbare Befehle ein.\n";
    std::cout << "> ";

    std::string input;
    std::string command;
    std::vector<std::string> params;

    while (true) {
        // Benutzerbefehl einlesen
        std::getline(std::cin, input);

        // Eingabe in Befehl und Parameter aufteilen
        std::istringstream iss(input);
        params.clear();

        // Ersten Teil als Befehl interpretieren
        iss >> command;

        // Restliche Teile als Parameter
        std::string param;
        while (iss >> param) {
            params.push_back(param);
        }

        // Befehl verarbeiten
        if (command == "stop" || command == "exit" || command == "quit") {
            break;  // Hauptschleife verlassen und Programm beenden
        }
        else if (command == "status") {
            ShowStatus(procMonitor, memMonitor, checksum, antiDebug);
        }
        else if (command == "help") {
            ShowHelp();
        }
        else if (command == "config") {
            ShowConfiguration();
        }
        else if (command == "addfile") {
            if (params.empty()) {
                std::cout << "Verwendung: addfile <pfad>\n";
            }
            else {
                checksum.AddFile(params[0]);
                std::cout << "Datei zur Überwachung hinzugefügt: " << params[0] << "\n";
            }
        }
        else if (command == "removefile") {
            if (params.empty()) {
                std::cout << "Verwendung: removefile <pfad>\n";
            }
            else {
                bool removed = checksum.RemoveFile(params[0]);
                if (removed) {
                    std::cout << "Datei von der Überwachung entfernt: " << params[0] << "\n";
                }
                else {
                    std::cout << "Datei nicht gefunden oder konnte nicht entfernt werden.\n";
                }
            }
        }
        else if (command == "checkfiles") {
            bool intact = checksum.VerifyFiles();
            if (intact) {
                std::cout << "Alle überwachten Dateien sind unverändert.\n";
            }
            else {
                std::cout << "WARNUNG: Es wurden Änderungen an überwachten Dateien festgestellt!\n";
            }
        }
        else if (command == "checkprocess") {
            if (params.empty()) {
                std::cout << "Verwendung: checkprocess <prozessname>\n";
            }
            else {
                DWORD pid = Common::GetProcessIdByName(params[0]);
                if (pid != 0) {
                    std::cout << "Prozess gefunden: " << params[0] << " (PID: " << pid << ")\n";
                }
                else {
                    std::cout << "Prozess nicht gefunden: " << params[0] << "\n";
                }
            }
        }
        else if (!command.empty()) {
            std::cout << "Unbekannter Befehl: " << command << "\n";
            std::cout << "Geben Sie 'help' ein für eine Liste der verfügbaren Befehle.\n";
        }

        // Prompt für nächste Eingabe
        std::cout << "> ";
    }

    //------------------------------------------------------------------------
    // 5. Herunterfahren
    //------------------------------------------------------------------------

    std::cout << "Anti-Cheat-System wird heruntergefahren...\n";

    // Alle Komponenten sauber beenden
    procMonitor.Stop();
    memMonitor.Stop();
    checksum.Stop();
    antiDebug.Stop();

    // Abschlussmeldung
    Common::Log("Anti-Cheat System beendet");
    std::cout << "Anti-Cheat-System beendet\n";

    return 0;
}
