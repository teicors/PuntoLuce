#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "configuration.h"

HttpServer server;

bool serverStarted = false;
//extern int Led0, Led2, Led4, Led5;
extern SwitchConfig SwitchCfg;
extern LampMessage LampMsg;
extern void saveConfig();
extern void sendData();
extern void flash_led0();
extern void flash_led2();
extern void flash_led4();
extern void flash_led5();

void onIndex(HttpRequest &request, HttpResponse &response)
{
        TemplateFileStream *tmpl = new TemplateFileStream("index.html");
        auto &vars = tmpl->variables();
        vars["Led0"] = SwitchCfg.Led0;
        vars["Led2"] = SwitchCfg.Led2;
        vars["Led4"] = SwitchCfg.Led4;
        vars["Led5"] = SwitchCfg.Led5;
        response.setAllowCrossDomainOrigin("*");
        response.sendTemplate(tmpl);
}

void onConfiguration(HttpRequest &request, HttpResponse &response)
{
        loadConfig();
        if (request.method == HTTP_POST)
        {
                debugf("Update config");
                // Update config
                if (request.getPostParameter("SSID").length() > 0) // Network
                {
                        SwitchCfg.NetworkSSID = request.getPostParameter("SSID");
                        SwitchCfg.NetworkPassword = request.getPostParameter("Password");
                }
                        
                SwitchCfg.Led0 = request.getPostParameter("Led0").toInt();
                SwitchCfg.Led2 = request.getPostParameter("Led2").toInt();
                SwitchCfg.Led4 = request.getPostParameter("Led4").toInt();
                SwitchCfg.Led5 = request.getPostParameter("Led5").toInt();

                saveConfig();
                startWebClock(); // Apply time zone settings
                response.headers[HTTP_HEADER_LOCATION] = "/";
        }

        debugf("Send template");
        TemplateFileStream *tmpl = new TemplateFileStream("config.html");
        auto &vars = tmpl->variables();
        vars["SSID"] = SwitchCfg.NetworkSSID;
        response.setAllowCrossDomainOrigin("*");
        response.sendTemplate(tmpl);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
        String file = request.uri.getRelativePath();
        if (file[0] == '/')
                file = file.substring(1);

        if (file[0] == '.')
                response.code = HTTP_STATUS_FORBIDDEN;
        else
        {
                response.setCache(86400, true); // It's important to use cache for better performance.
                response.setAllowCrossDomainOrigin("*");
                response.sendFile(file);
        }
}

/// API ///

void onApiDoc(HttpRequest &request, HttpResponse &response)
{
        TemplateFileStream *tmpl = new TemplateFileStream("api.html");
        auto &vars = tmpl->variables();
        vars["IP"] = (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
        response.setAllowCrossDomainOrigin("*");
        response.sendTemplate(tmpl);
}

void onApiSensors(HttpRequest &request, HttpResponse &response)
{
        JsonObjectStream* stream = new JsonObjectStream();
        JsonObject& json = stream->getRoot();
        json["status"] = (bool)true;
        JsonObject& sensors = json.createNestedObject("sensors");
        sensors["Led0"] = SwitchCfg.Led0;
        sensors["Led2"] = SwitchCfg.Led2;
        sensors["Led4"] = SwitchCfg.Led4;
        sensors["Led5"] = SwitchCfg.Led5;
        response.setAllowCrossDomainOrigin("*");
        response.sendDataStream(stream, MIME_JSON);
}

void onReboot(HttpRequest &request, HttpResponse &response)
{
        System.restart();
}

void onApiOutput(HttpRequest &request, HttpResponse &response)
{
        int val = request.getQueryParameter("valore").toInt();
        int led = request.getQueryParameter("led").toInt();
        debugf("valore: %d", val);
        debugf("led: %d", led);
        if (val == 0 || val == 1)
        {    
            if (led == 0)
            {
//                Led0=val;
                SwitchCfg.Led0=val;
                flash_led0();
            }
            else if (led == 2)
            {
//                Led2=val;
                SwitchCfg.Led2=val;
                flash_led2();
            }
            else if (led == 4)
            {
//                Led4=val;
                SwitchCfg.Led4=val;
                flash_led4();
            }
            else if (led == 5)
            {
//                Led5=val;
                SwitchCfg.Led5=val;
                flash_led5();
            }
        }
        else
                val = -1;
//        Serial.printf("led %d control %d",led,val);
        JsonObjectStream* stream = new JsonObjectStream();
        JsonObject& json = stream->getRoot();
        json["status"] = val != -1;
        if (val == -1) json["error"] = "Wrong control parameter value, please use: ?control=0|1";
        response.setAllowCrossDomainOrigin("*");
        response.sendDataStream(stream, MIME_JSON);
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
        
}

void onApiSwitch(HttpRequest &request, HttpResponse &response)
{
        int val = request.getPostParameter("switch").toInt();
        
        LampMsg.evento = LIGHT;
        LampMsg.pulsante = val;
        LampMsg.stato = 0;
        LampMsg.valore = 1;
        saveConfig();
        sendData();
}

void onApiStatus(HttpRequest &request, HttpResponse &response)
{
    JsonObjectStream* stream = new JsonObjectStream();
    JsonObject& json = stream->getRoot();
    json["status"] = (bool)true;
    JsonObject& sensors = json.createNestedObject("sensors");
    sensors["Switch powered"] = SwitchCfg.powered;
    sensors["Switch Switch0"] = SwitchCfg.Led0;
    sensors["Switch Switch2"] = SwitchCfg.Led2;
    sensors["Switch Switch4"] = SwitchCfg.Led4;
    sensors["Switch Switch5"] = SwitchCfg.Led5;
    response.sendDataStream(stream, MIME_JSON);
}

void OnApiShowInfo(HttpRequest &request, HttpResponse &response) {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}


void startWebServer()
{
    if (serverStarted) return;

    server.listen(80);
    server.paths.set("/", onIndex);
    server.paths.set("/api", onApiDoc);
    server.paths.set("/api/sensors", onApiSensors);
    server.paths.set("/api/output", onApiOutput);
    server.paths.set("/api/swswitch", onApiSwitch);
    server.paths.set("/config", onConfiguration);
    server.paths.set("/reboot", onReboot);
    server.paths.set("/api/ShowInfo", OnApiShowInfo);    
    server.paths.set("/api/status", onApiStatus);
    server.paths.setDefault(onFile);
    serverStarted = true;

    if (WifiStation.isEnabled())
            debugf("STA: %s", WifiStation.getIP().toString().c_str());
    if (WifiAccessPoint.isEnabled())
        debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

/// FileSystem Initialization ///

Timer downloadTimer;
HttpClient downloadClient;
int dowfid = 0;
void downloadContentFiles()
{
        debugf("DownloadContentFiles");

        downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoControl.html", "index.html");
        downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoConfig.html", "config.html");
        downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoAPI.html", "api.html");
        downloadClient.downloadFile("http://simple.anakod.ru/templates/bootstrap.css.gz");
        downloadClient.downloadFile("http://simple.anakod.ru/templates/jquery.js.gz", (RequestCompletedDelegate)([](HttpConnection& connection, bool success) -> int {
                if(success) {
                        startWebServer();
                }
        }));
}
