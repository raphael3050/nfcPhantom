#include <Arduino.h>
#include <Wire.h>
#include <OneButton.h>
#include <TFT_eSPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Adafruit_PN532.h>
#include <boot_img.h>
#define PIN_POWER_ON 15
#define PIN_BUTTON_1 0
#define PIN_BUTTON_2 14

int sda = 18;
int scl = 17;
int start_read = 0;
volatile bool connected = false;

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
    button_start_read.attachClick(startRecordClick);
    // Setup the display, print the logo for 1 second and then clear the screen

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(0, 0, 320, 170, (uint16_t *)epd_bitmap_Frame_1);

    // Clear the screen after 1 second
    delay(2000);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);

    nfc.begin();
    delay(100);

    nfc.begin();
    nfc.SAMConfig();
    displayMessage("[+] NFC reader started, waiting for a tag...");
}

void loop()
{
    if (start_read == 1)
    {
        uint8_t success;
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer pour l'UID de la carte
        uint8_t uidLength;                     // Taille de l'UID
        char buffer[32];                       // for the itoa function

        // Essayer de lire l'UID de la carte NFC
        nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

        if (success)
        {
            displayMessage("[+] Found an NFC card!");
            displayMessage("[+] UID Length: ", false);
            displayMessage(itoa(uidLength, buffer, 10), false);
            displayMessage(" bytes");
            Serial.print("[+] UID Value: ");
            displayMessage("[+] UID Value: ", false);
            for (uint8_t i = 0; i < uidLength; i++)
            {
                displayMessage(" 0x", (boolean) false);
                displayMessage(itoa(uid[i], buffer, 16), false);
            }
            Serial.println("");
            // Clé d'authentification par défaut A pour le bloc 4
            uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            // Authentifier le bloc 4
            success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyA);
            if (success)
            {
                displayMessage("[+] Authenticated block 4");

                // Lire le bloc 4
                uint8_t data[16];
                success = nfc.mifareclassic_ReadDataBlock(4, data);
                if (success)
                {
                    displayMessage("[+] Read block 4:");
                    for (uint8_t i = 0; i < 16; i++)
                    {
                        displayMessage(" 0x", false);
                        displayMessage(itoa(data[i], buffer, 16), false);
                    }
                    Serial.println("");
                }
                else
                {
                    displayMessage("[-] Failed to read block 4");
                }
            }
            else
            {
                displayMessage("[-] Authentication failed for block 4");
            }
            delay(1000);
        }
    }
    button_start_read.tick();
    delay(100);
}
