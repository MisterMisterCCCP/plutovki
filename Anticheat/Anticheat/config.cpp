// config.cpp - Implementierung des Konfigurationsmanagements
#include "config.h"
#include "common.h"
#include <fstream>
#include <sstream>

// Statische Initialisierung
std::map<std::string, std::map<std::string, std::string>> Config::configData;

bool Config::Load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Common::Log("Konnte Konfigurationsdatei nicht öffnen: " + filename);
        return false;
    }

    configData.clear();
    std::string line;
    std::string currentSection = "General";

    while (std::getline(file, line)) {
        // Leerzeilen und Kommentare überspringen
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) {
            continue;
        }

        // Abschnitt
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Schlüssel-Wert-Paar
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Whitespace trimmen
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            configData[currentSection][key] = value;
        }
    }

    Common::Log("Konfigurationsdatei geladen: " + filename);
    return true;
}

bool Config::Save(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        Common::Log("Konnte Konfigurationsdatei nicht speichern: " + filename);
        return false;
    }

    for (const auto& section : configData) {
        file << "[" << section.first << "]\n";
        
        for (const auto& keyValue : section.second) {
            file << keyValue.first << " = " << keyValue.second << "\n";
        }
        
        file << "\n";
    }

    Common::Log("Konfigurationsdatei gespeichert: " + filename);
    return true;
}

std::string Config::GetString(const std::string& section, const std::string& key, 
                             const std::string& defaultValue) {
    auto sectionIt = configData.find(section);
    if (sectionIt != configData.end()) {
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            return keyIt->second;
        }
    }
    return defaultValue;
}

int Config::GetInt(const std::string& section, const std::string& key, int defaultValue) {
    std::string value = GetString(section, key, "");
    if (!value.empty()) {
        try {
            return std::stoi(value);
        }
        catch (...) {
            // Fehler beim Umwandeln, Standardwert zurückgeben
        }
    }
    return defaultValue;
}

bool Config::GetBool(const std::string& section, const std::string& key, bool defaultValue) {
    std::string value = GetString(section, key, "");
    if (!value.empty()) {
        return (value == "1" || value == "true" || value == "yes" || value == "on");
    }
    return defaultValue;
}

std::vector<std::string> Config::GetList(const std::string& section, const std::string& key) {
    std::vector<std::string> result;
    std::string value = GetString(section, key, "");
    
    if (!value.empty()) {
        std::stringstream ss(value);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            // Whitespace trimmen
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            
            if (!item.empty()) {
                result.push_back(item);
            }
        }
    }
    
    return result;
}

void Config::SetString(const std::string& section, const std::string& key, const std::string& value) {
    configData[section][key] = value;
}

void Config::SetInt(const std::string& section, const std::string& key, int value) {
    configData[section][key] = std::to_string(value);
}

void Config::SetBool(const std::string& section, const std::string& key, bool value) {
    configData[section][key] = value ? "true" : "false";
}

void Config::SetList(const std::string& section, const std::string& key, 
                    const std::vector<std::string>& values) {
    std::string result;
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            result += ", ";
        }
        result += values[i];
    }
    
    configData[section][key] = result;
}
