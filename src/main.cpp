#include <Arduino.h>
#include <Wire.h>
#include <OneButton.h>
#include <TFT_eSPI.h>
#include <PN532.h>

#include <NfcAdapter.h>
#include <Adafruit_PN532.h>

#define PIN_POWER_ON 15
#define PIN_BUTTON_1 0
#define PIN_BUTTON_2 14

int sda = 18;
int scl = 17;
int message_displayed = 0;
int start_read = 0;
volatile bool connected = false;

OneButton button(PIN_BUTTON_1, true);
OneButton button_start_read(PIN_BUTTON_2, true);
TFT_eSPI tft = TFT_eSPI();
Adafruit_PN532 nfc(sda, scl);


void displayMessage(char const *message, boolean newline = true)
{   
    if (newline)
    {
        Serial.println(message);
        tft.println(message);
    }
    else
    {
        tft.print(message);
        Serial.print(message);
    }
}


void deviceScan(TwoWire *_port, Stream *stream)
{
    uint8_t err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++)
    {
        _port->beginTransmission(addr);
        err = _port->endTransmission();
        if (err == 0)
        {
            stream->print("[+] I2C device found at address 0x");
            tft.print("[+] I2C device found at address 0x");
            if (addr < 16)
            {
                stream->print("0");
                tft.print("0");
            }
            stream->print(addr, HEX);
            stream->println(" !");
            tft.print(addr, HEX);
            tft.println(" !");
            nDevices++;
        }
        else if (err == 4)
        {
            stream->print("[+] Unknow error at address 0x");
            if (addr < 16)
                stream->print("0");
            stream->println(addr, HEX);
        }
    }
    if (nDevices == 0)
    {
        stream->println("[+] No I2C devices found\n");
        tft.println("[+] No I2C devices found\n");
    }
    else
    {
        stream->println("[+] Done\n");
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("[+] Done\n");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
}

void simpleClick()
{
    displayMessage("[+] Scanning I2C devices...");
    deviceScan(&Wire, &Serial);
}

void startRecordClick()
{
    displayMessage("[+] Scanning NFC devices...");
    start_read = 1;
}

void setup()
{
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    Serial.begin(115200);
    //Wire.begin(sda, scl);

    button.attachClick(simpleClick);
    button_start_read.attachClick(startRecordClick);
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);

    nfc.begin();
    nfc.SAMConfig();
    displayMessage("[+] NFC reader started, waiting for a tag...");
}

void loop()
{
    if (message_displayed == 0)
    {
        displayMessage("[+] Press the button to start scanning I2C devices...");
        message_displayed = 1;
    }
    if (start_read == 1)
    {
        uint8_t success;
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer pour l'UID de la carte
        uint8_t uidLength;                     // Taille de l'UID
        char buffer[32]; // for the itoa function

        // Essayer de lire l'UID de la carte NFC
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        if (success)
        {
            displayMessage("[+] Found an NFC card!");
            // Afficher l'UID de la carte NFC
            displayMessage("[+] UID Length: ", false);

            displayMessage(itoa(uidLength, buffer, 10), false);            
            displayMessage(" bytes");
            Serial.print("[+] UID Value: ");
            displayMessage("[+] UID Value: ", false);
            for (uint8_t i = 0; i < uidLength; i++)
            {
                displayMessage(" 0x", (boolean)false);
                Serial.print(uid[i], HEX);
            }
            Serial.println("");

            delay(1000);
        }
    }
    button.tick();
    button_start_read.tick();
    delay(100);
}
