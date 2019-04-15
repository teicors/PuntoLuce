#include "../include/configuration.h"

#include <SmingCore.h>

extern  SwitchConfig SwitchCfg;

void loadConfig()
{
        DynamicJsonBuffer jsonBuffer;
        if (fileExist(SWITCH_CONFIG_FILE))
        {
                int size = fileGetSize(SWITCH_CONFIG_FILE);
                char* jsonString = new char[size + 1];
                fileGetContent(SWITCH_CONFIG_FILE, jsonString, size + 1);
                JsonObject& root = jsonBuffer.parseObject(jsonString);

                SwitchCfg.NetworkSSID = String((const char*)root["ssid"]);
                SwitchCfg.NetworkPassword = String((const char*)root["password"]);

                
                SwitchCfg.Led0 = root["Switch0"];
                SwitchCfg.Led2 = root["Switch2"];
                SwitchCfg.Led4 = root["Switch4"];
                SwitchCfg.Led5 = root["Switch5"];

                delete[] jsonString;
        }
        else
        {
                SwitchCfg.NetworkSSID = WIFI_SSID;
                SwitchCfg.NetworkPassword = WIFI_PWD;
        }
//        return cfg;
}

void saveConfig()
{
//        ActiveConfig = cfg;

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();

        JsonObject& network = jsonBuffer.createObject();
        root["ssid"] = SwitchCfg.NetworkSSID.c_str();
        root["password"] = SwitchCfg.NetworkPassword.c_str();
        
        root["Switch0"] = SwitchCfg.Led0;
        root["Switch2"] = SwitchCfg.Led2;
        root["Switch4"] = SwitchCfg.Led4;
        root["Switch5"] = SwitchCfg.Led5;

        char buf[3048];
        root.prettyPrintTo(buf, sizeof(buf));
        fileSetContent(SWITCH_CONFIG_FILE, buf);
}


