// config.h - Konfigurationsmanagement für das Anti-Cheat-System
//
// Diese Datei definiert die Funktionen zum Laden und Speichern
// von Konfigurationseinstellungen aus einer externen Datei.

#pragma once
#include <string>
#include <vector>
#include <map>

class Config {
public:
    /**
     * Lädt die Konfigurationseinstellungen aus der angegebenen Datei.
     * 
     * @param filename Pfad zur Konfigurationsdatei
     * @return true bei erfolgreichem Laden, false bei Fehler
     */
    static bool Load(const std::string& filename = "config.ini");
    
    /**
     * Speichert die aktuellen Konfigurationseinstellungen in die angegebene Datei.
     * 
     * @param filename Pfad zur Konfigurationsdatei
     * @return true bei erfolgreichem Speichern, false bei Fehler
     */
    static bool Save(const std::string& filename = "config.ini");
    
    /**
     * Gibt den Wert einer String-Einstellung zurück.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param defaultValue Der Standardwert, falls die Einstellung nicht existiert
     * @return Der Wert der Einstellung oder defaultValue
     */
    static std::string GetString(const std::string& section, const std::string& key, 
                                const std::string& defaultValue = "");
    
    /**
     * Gibt den Wert einer Integer-Einstellung zurück.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param defaultValue Der Standardwert, falls die Einstellung nicht existiert
     * @return Der Wert der Einstellung oder defaultValue
     */
    static int GetInt(const std::string& section, const std::string& key, int defaultValue = 0);
    
    /**
     * Gibt den Wert einer Boolean-Einstellung zurück.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param defaultValue Der Standardwert, falls die Einstellung nicht existiert
     * @return Der Wert der Einstellung oder defaultValue
     */
    static bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false);
    
    /**
     * Gibt eine Liste von Werten zurück.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @return Liste der Werte oder leere Liste
     */
    static std::vector<std::string> GetList(const std::string& section, const std::string& key);
    
    /**
     * Setzt den Wert einer String-Einstellung.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param value Der neue Wert
     */
    static void SetString(const std::string& section, const std::string& key, const std::string& value);
    
    /**
     * Setzt den Wert einer Integer-Einstellung.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param value Der neue Wert
     */
    static void SetInt(const std::string& section, const std::string& key, int value);
    
    /**
     * Setzt den Wert einer Boolean-Einstellung.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param value Der neue Wert
     */
    static void SetBool(const std::string& section, const std::string& key, bool value);
    
    /**
     * Setzt eine Liste von Werten.
     * 
     * @param section Der Abschnitt in der Konfigurationsdatei
     * @param key Der Name der Einstellung
     * @param values Liste der Werte
     */
    static void SetList(const std::string& section, const std::string& key, 
                       const std::vector<std::string>& values);

private:
    static std::map<std::string, std::map<std::string, std::string>> configData;
};
