#include <user_config.h>
#include <SmingCore/SmingCore.h>

///////////////////////////////////////////////////////////////////
// Set your SSID & Pass for initial configuration
#include "configuration.h" // application configuration
///////////////////////////////////////////////////////////////////
// wget "http://192.168.1.220/api/output?status=1&led=2" -o pippo
///////////////////////////////////////////////////////////////////
#include "special_chars.h"
#include "webserver.h"

#define say(a) ( Serial.print(a) )

#define INT_PIN0 0   // GPIO0 d3
#define INT_PIN2 2   // GPIO2 d4
#define INT_PIN4 4   // GPIO4 d2
#define INT_PIN5 5   // GPIO5 d1
#define INT_PIN14 14 // GPIO14 d5
#define INT_PIN12 12 // GPIO12 d6
#define INT_PIN13 13 // GPIO13 d7
#define INT_PIN15 15 // GPIO15 d8

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
//#ifndef WIFI_SSID
//	#define WIFI_SSID "angelo-U36SG" // Put you SSID and Password here
//	#define WIFI_PWD "L4R7qluW"
//#endif

#define MAX_READ 41

#define JSON_HOST "10.42.0.1"
#define JSON_PORT 5008

Timer Timer0;
Timer Timer2;
Timer Timer4;
Timer Timer5;
Timer Timerl0;
Timer Timerl2;
Timer Timerl4;
Timer Timerl5;
Timer procTimer;
Timer flashled;

int Led0, Led2, Led4, Led5;
int flash0, flash2, flash4, flash5;
int inter0, inter2, inter4, inter5;
int volte0, volte2, volte4, volte5;
int data_to_send0, data_to_send2, data_to_send4, data_to_send5;
unsigned long last0, last2, last4, last5;
String String2;
SwitchConfig SwitchCfg;
LampMessage LampMsg;

#define FTP_SERVER 1

#if FTP_SERVER == 1
FtpServer ftp;
#endif

// Forward declarations
//void publishMessage(int led, int numero);
void sendData();

void JsonOnComplete(TcpClient& client, bool successful)
{
        // debug msg
        debugf("JsonOnComplete");
        debugf("successful: %d", successful);
        client.close();
}

void JsonOnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
        // debug msg
        debugf("JsonOnReadyToSend");
        debugf("sourceEvent: %d", sourceEvent);

        // The connection is made at the time of shipment
        if(sourceEvent == eTCE_Connected)
        {
        DynamicJsonBuffer jsonBuffer;
        JsonObject& MsgToSend = jsonBuffer.createObject();

        MsgToSend["ip"]        = WifiStation.getIP().toString();
        
        MsgToSend["pulsante"]  = LampMsg.pulsante;
        MsgToSend["evento"]    = LampMsg.evento;
        MsgToSend["stato"]     = LampMsg.stato;
        MsgToSend["valore"]    = LampMsg.valore;

//        MsgToSend["slenabled"] = LampCfg.sleepenabled;
//        MsgToSend["alenabled"] = LampCfg.alarmenabled;
//        MsgToSend["buenabled"] = LampCfg.buzzerenabled;
//        MsgToSend["alarmtime"] = LampCfg.alarmtime;
//        MsgToSend["sleeptime"] = LampCfg.sleeptime;
//        MsgToSend["powered"]   = LampCfg.powered;
        
        client.sendString("POST /json HTTP/1.1\r\n");
        client.sendString("Accept: */*\r\n");
        client.sendString("Content-Type: application/json;charset=utf-8\r\n");
        client.sendString("Content-Length: "+String(MsgToSend.measureLength()+1)+"\r\n");
        client.sendString("\r\n");
        char buf[MsgToSend.measureLength()];
        MsgToSend.printTo(buf, sizeof(buf)+1);
        client.sendString(buf);
        MsgToSend.printTo(Serial);
        client.sendString("\r\n");
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
        }
}

bool JsonOnReceive(TcpClient& client, char *buf, int size)
{
        // debug msg
        debugf("JsonOnReceive");
        debugf("%s", buf);
        client.close();
}

// Create an object of class JsonMon TcpClient
TcpClient JsonMon(JsonOnComplete, JsonOnReadyToSend, JsonOnReceive);



// This function will be called by timer
void sendData()
{
        // We read the sensor data
        // connect to the server narodmon
        JsonMon.connect(JSON_HOST, JSON_PORT);
}


void flash_led0() {
    if ( flash0 == true) {
        if (volte0<30) {
            digitalWrite(INT_PIN15, (volte0>>1)&1);
        }
        else 
        {
            digitalWrite(INT_PIN15, volte0%2);
        }
    }
    else
    {
        digitalWrite(INT_PIN15, SwitchCfg.Led0);
    }
}

void flash_led2() {
    if ( flash2 == true) {
        if (volte2<30) {
            digitalWrite(INT_PIN14, (volte2>>1)&1);
        }
        else 
        {
            digitalWrite(INT_PIN14, volte2%2);
        }
    }
    else
    {
        digitalWrite(INT_PIN14, SwitchCfg.Led2);
    }
}

void flash_led4() {
    if ( flash4 == true) {
        if (volte4<30) {
            digitalWrite(INT_PIN13, (volte4>>1)&1);
        }
        else 
        {
            digitalWrite(INT_PIN13, volte4%2);
        }
    }
    else
    {
        digitalWrite(INT_PIN13, SwitchCfg.Led4);
    }
}

void flash_led5() {
    if ( flash5 == true) {
        if (volte5<30) {
            digitalWrite(INT_PIN12, (volte5>>1)&1);
        }
        else 
        {
            digitalWrite(INT_PIN12, volte5%2);
        }
    }
    else
    {
        digitalWrite(INT_PIN12, SwitchCfg.Led5);
    }
}


void check_button0() {
    if (inter0==true) {   
        flash0=true;
        if (volte0<MAX_READ) {
            volte0++;
        }
    }
    if (data_to_send0==1) {
        flash0=false;
//        publishMessage(0,volte0);
        LampMsg.evento=LIGHT;
        LampMsg.stato=Pulsantiera_4_posti;
        LampMsg.valore=volte0;
        LampMsg.pulsante=0;
        sendData();
        data_to_send0=0;
    }
}

void check_button2() {
    if (inter2==true) {   
        flash2=true;
        if (volte2<MAX_READ) {
            volte2++;
        }
    }
    if (data_to_send2==1) {
        flash0=false;
//        publishMessage(2,volte2);
        LampMsg.evento=LIGHT;
        LampMsg.stato=Pulsantiera_4_posti;
        LampMsg.valore=volte2;
        LampMsg.pulsante=2;
        sendData();
        data_to_send2=0;
    }
}

void check_button4() {
    if (inter4==true) {   
        flash4=true;
        if (volte4<MAX_READ) {
            volte4++;
        }
    }
    if (data_to_send4==1) {
        flash4=false;
//        publishMessage(4,volte4);
        LampMsg.evento=LIGHT;
        LampMsg.stato=Pulsantiera_4_posti;
        LampMsg.valore=volte4;
        LampMsg.pulsante=4;
        sendData();
        data_to_send4=0;
    }
}

void check_button5() {
    if (inter5==true) {   
        flash5=true;
        if (volte5<MAX_READ) {
            volte5++;
        }
    }
    if (data_to_send5==1) {
        flash5=false;
//        publishMessage(5,volte5);
        LampMsg.evento=LIGHT;
        LampMsg.stato=Pulsantiera_4_posti;
        LampMsg.valore=volte5;
        LampMsg.pulsante=5;
        sendData();
        data_to_send5=0;
    }
}


void IRAM_ATTR interruptHandler01();

void IRAM_ATTR interruptHandler00()
{
    inter0=true;
    volte0=0;
    Serial.print("Press button0\n");
    detachInterrupt(INT_PIN0);
    attachInterrupt(INT_PIN0, interruptHandler01, RISING);
}

void IRAM_ATTR interruptHandler01()
{
    inter0=false;
    flash0=false;
    Serial.print("Release button0\n");
    detachInterrupt(INT_PIN0);
    attachInterrupt(INT_PIN0, interruptHandler00, FALLING);
    data_to_send0=1;
}        

void IRAM_ATTR interruptHandler21();

void IRAM_ATTR interruptHandler20()
{
    inter2=true;
    volte2=0;
    Serial.print("Press button2\n");
    detachInterrupt(INT_PIN2);
    attachInterrupt(INT_PIN2, interruptHandler21, RISING);
}

void IRAM_ATTR interruptHandler21()
{
    inter2=false;
    flash2=false;
    Serial.print("Release button2\n");
    detachInterrupt(INT_PIN2);
    attachInterrupt(INT_PIN2, interruptHandler20, FALLING);
    data_to_send2=1;
}        

void IRAM_ATTR interruptHandler41();

void IRAM_ATTR interruptHandler40()
{
    inter4=true;
    volte4=0;
    Serial.print("Press button4\n");
    detachInterrupt(INT_PIN4);
    attachInterrupt(INT_PIN4, interruptHandler41, RISING);
}

void IRAM_ATTR interruptHandler41()
{
    inter4=false;
    flash4=false;
    Serial.print("Release button4\n");
    detachInterrupt(INT_PIN4);
    attachInterrupt(INT_PIN4, interruptHandler40, FALLING);
    data_to_send4=1;
}        

void IRAM_ATTR interruptHandler51();

void IRAM_ATTR interruptHandler50()
{
    inter5=true;
    volte5=0;
    Serial.print("Press button5\n");
    detachInterrupt(INT_PIN5);
    attachInterrupt(INT_PIN5, interruptHandler51, RISING);
}

void IRAM_ATTR interruptHandler51()
{
    inter5=false;
    flash5=false;
    Serial.print("Release button5\n");
    detachInterrupt(INT_PIN5);
    attachInterrupt(INT_PIN5, interruptHandler50, FALLING);
    data_to_send5=1;
}        


void startFTP()
{
        if (!fileExist("index.html"))
                fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

        // Start FTP server
        ftp.listen(21);
        ftp.addUser("me", "123"); // FTP account
}


void flashleds()
{
        digitalWrite(INT_PIN12, 1);
        digitalWrite(INT_PIN13, 1);
        digitalWrite(INT_PIN14, 1);
        digitalWrite(INT_PIN15, 1);
        delay(50);
        digitalWrite(INT_PIN12, 0);
        digitalWrite(INT_PIN13, 0);
        digitalWrite(INT_PIN14, 0);
        digitalWrite(INT_PIN15, 0);
        delay(50);
        digitalWrite(INT_PIN12, 1);
        digitalWrite(INT_PIN13, 1);
        digitalWrite(INT_PIN14, 1);
        digitalWrite(INT_PIN15, 1);
        delay(50);    
        digitalWrite(INT_PIN12, 0);
        digitalWrite(INT_PIN13, 0);
        digitalWrite(INT_PIN14, 0);
        digitalWrite(INT_PIN15, 0);
        delay(50);
}


void SendPresence()
{
//    publishMessage(WifiStation.getIP().toString().c_str(),9999,-1);
    LampMsg.evento=SEND_PRESENCE;
    LampMsg.stato=Pulsantiera_4_posti;
    LampMsg.valore=0;
    LampMsg.pulsante=0;
    sendData();
}

// Will be called when WiFi station was connected to AP
void connectOk(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t channel)
{
    debugf("I'm CONNECTED");
    WifiAccessPoint.enable(false);
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
        Serial.println(WifiStation.getIP().toString());
        
//        startMqttClient();
        
        Timer0.initializeMs(50, check_button0).start();
        Timer2.initializeMs(50, check_button2).start();
        Timer4.initializeMs(50, check_button4).start();
        Timer5.initializeMs(50, check_button5).start();

        Timerl0.initializeMs(50, flash_led0).start();
        Timerl2.initializeMs(50, flash_led2).start();
        Timerl4.initializeMs(50, flash_led4).start();
        Timerl5.initializeMs(50, flash_led5).start();
        


        debugf("connected");

        // At first run we will download web server content
        //if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("api.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
        //    downloadContentFiles();
        //else
            startWebServer();
        #if FTP_SERVER == 1
            startFTP();
        #endif

        flashled.initializeMs(500, flashleds).stop();
        SendPresence();
}

// Will be called when WiFi station timeout was reached
void connectFail(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
        debugf("I'm NOT CONNECTED!");
	WifiAccessPoint.config("LampConfig", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);

	startWebServer();

        WifiStation.disconnect();
        WifiStation.connect();
}


void init()
{
        spiffs_mount(); // Mount file system, in order to work with files
        Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default
        flashled.initializeMs(500, flashleds).start();

        delay(3000);
        say("======= Bring GPIO low to trigger interrupt(s) =======");
        Serial.println();

        pinMode(INT_PIN12, OUTPUT);
        pinMode(INT_PIN13, OUTPUT);
        pinMode(INT_PIN14, OUTPUT);
        pinMode(INT_PIN15, OUTPUT);

        pinMode(INT_PIN0, INPUT);
        pinMode(INT_PIN2, INPUT);
        pinMode(INT_PIN4, INPUT);
        pinMode(INT_PIN5, INPUT);

       	attachInterrupt(INT_PIN0, interruptHandler00, FALLING);
        attachInterrupt(INT_PIN2, interruptHandler20, FALLING);
        attachInterrupt(INT_PIN4, interruptHandler40, FALLING);
        attachInterrupt(INT_PIN5, interruptHandler50, FALLING);


        loadConfig();
        
        // Station - WiFi client
//        WifiStation.enable(true);
//        WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
//       	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
        
        WifiEvents.onStationConnect(connectOk);
        WifiEvents.onStationDisconnect(connectFail);
        WifiEvents.onStationGotIP(gotIP);

}

////// WEB Clock //////
Timer clockRefresher;
HttpClient clockWebClient;
uint32_t lastClockUpdate = 0;
DateTime clockValue;
const int clockUpdateIntervalMs = 10 * 60 * 1000; // Update web clock every 10 minutes

int onClockUpdating(HttpConnection& client, bool successful)
{
        if(!successful) {
                debugf("CLOCK UPDATE FAILED %d (code: %d)", successful, client.getResponse()->code);
                lastClockUpdate = 0;
                return -1;
        }

        // Extract date header from response
        clockValue = client.getResponse()->headers.getServerDate();
        if(clockValue.isNull())
                clockValue = client.getResponse()->headers.getLastModifiedDate();
//        if(!clockValue.isNull())
//                clockValue.addMilliseconds(ActiveConfig.AddTZ * 1000 * 60 * 60);
        return 0;
}

void refreshClockTime()
{
        uint32_t nowClock = millis();
        String StrTime;
        if(nowClock < lastClockUpdate)
                lastClockUpdate = 0; // Prevent overflow, restart
        if((lastClockUpdate == 0 || nowClock - lastClockUpdate > clockUpdateIntervalMs)) {
                clockWebClient.downloadString("google.com", onClockUpdating);
                lastClockUpdate = nowClock;
        } else if(!clockValue.isNull())
                clockValue.addMilliseconds(clockRefresher.getIntervalMs());

        if(!clockValue.isNull()) {
                StrTime = clockValue.toShortDateString() + " " + clockValue.toShortTimeString(false);

                if((millis() % 2000) > 1000)
                        StrTime.setCharAt(13, ' ');
                else
                        StrTime.setCharAt(13, ':');
        }
}

void startWebClock()
{
        lastClockUpdate = 0;
        clockRefresher.stop();
        clockRefresher.initializeMs(500, refreshClockTime).start();
}
