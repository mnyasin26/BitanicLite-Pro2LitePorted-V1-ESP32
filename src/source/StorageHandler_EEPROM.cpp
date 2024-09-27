#include "header/StorageHandler_EEPROM.h"
#include "header/timeHandler.h"

extern TimeHandler time_h;

StorageHandler_EEPROM::StorageHandler_EEPROM()
{
}

void StorageHandler_EEPROM::startEEPROM(int size)
{
    EEPROM.begin(size);
}

void StorageHandler_EEPROM::saveConfig(StaticJsonDocument<JSON_SIZE> doc)
{
    EEPROM.begin(4000);

    String date = time_h.getDateWithCalc(doc["jadwal"]["minggu"].as<int>());
    DEBUG_PRINTLN(date);
    doc["date"] = date;

    String test;
    serializeJson(doc, test);
    // DEBUG_PRINTLN(test);

    EEPROM.writeString(2500, test);
    // DEBUG_PRINTLN(test);

    EEPROM.commit();
    EEPROM.end();
    // Close the file
}

StaticJsonDocument<JSON_SIZE> StorageHandler_EEPROM::loadConfig()
{
    EEPROM.begin(4000);
    // Open file for reading
    StaticJsonDocument<JSON_SIZE> doc;
    String test;
    test = EEPROM.readString(2500);
    // DEBUG_PRINTLN(test);
    DeserializationError error = deserializeJson(doc, test);
    if (error)
    {
        DEBUG_PRINTLN(F("Failed to read file, using default configuration"));
        doc["status"] = "error";
        return doc;
    }

    // DEBUG_PRINTLN(doc["ssid"].as<String>());
    // EEPROM.end();
    return doc;
    // Extract each characters by one by one
    // while (file.available()) {
    //   DEBUG_PRINT((char)file.read());
    // }
    // DEBUG_PRINTLN();

    // Close the file
    EEPROM.end();
}