// SDCard.cpp
#include "sd-card.h"

SDCard::SDCard()
    : _cardPresent(false),
      _lastDebounce(0),
      _pendingEvent(false)
{
    // Create SPI mutex once
    pinMode(cdPin, INPUT_PULLUP);
    // Attach interrupt for card detect
    attachInterrupt(cdPin, &SDCard::cardDetectISR, this, CHANGE);
    // Record initial state
    _cardPresent = sdCardPresent();
}

SDCard::~SDCard()
{
    // Nothing to free
}

void SDCard::cardDetectISR()
{
    // Debounce logic
    uint32_t now = millis();
    if (now - _lastDebounce > 50) // 50 ms debounce time
    {
        _lastDebounce = now;
        _cardPresent = !digitalRead(cdPin); // LOW means card present
        _pendingEvent = true;               // Set flag to handle in main loop
    }
}

bool SDCard::sdCardPresent() const
{
    // If the card-detect switch is active-low:
    return (digitalRead(cdPin) == LOW);
}

bool SDCard::guardedBegin()
{
    SPI.begin(); // Ensure SPI is powered up
    bool ok = sd.begin(chipSelect, SD_SCK_MHZ(25));
    return ok;
}

bool SDCard::init()
{
    // Called when you want to ensure SD.begin() is called once the card is present
    if (initialized && _cardPresent)
        return true;

    if (!sdCardPresent())
    {
        Log.error("SDCard not present");
        return false;
    }

    if (!guardedBegin())
    {
        Log.error("SDCard initialization failed");
        initialized = false;
        return false;
    }
    Log.info("SDCard initialized successfully");
    initialized = true;
    return true;
}

bool SDCard::ready()
{
    // True if card is inserted AND we have successfully called init()
    return sdCardPresent() && initialized;
}

String SDCard::read(const String &path, unsigned long &startPoint, char terminatingChar)
{
    String result;
    if (!init())
    {
        Log.error("SDCard not initialized or card not present");
        return result;
    }

    SdFile file;
    if (file.open(path.c_str(), O_READ) && file.seekSet(startPoint))
    {
        int c;
        while ((c = file.read()) >= 0)
        {
            startPoint++;
            if (c == terminatingChar)
                break;
            result += char(c);
        }
        file.close();
    }
    return result;
}

bool SDCard::overwrite(const char *path, const char *newContent)
{
    if (!init())
        return false;

    SdFile file;
    bool ok = file.open(path, O_WRONLY | O_CREAT | O_TRUNC);
    if (ok)
    {
        file.print(newContent);
        file.close();
    }
    return ok;
}

uint64_t SDCard::appendln(const String &path, const String &message)
{

    if (!init())
        return 0;

    SdFile file;
    uint64_t size = 0;
    if (file.open(path.c_str(), O_RDWR | O_CREAT | O_AT_END))
    {
        file.println(message);
        size = file.fileSize();
        file.close();
    }
    return size;
}

// void SDCard::bridgeSpi()
// {
//     if (!spiMutex)
//         return;
//     SPI.begin();
//     SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
//     SPI.endTransaction();
// }

// -------------------------------------------------------------
//  Card Detect ISR & Debounce Logic
// -------------------------------------------------------------

// void SDCard::beginCardDetectInterrupt()
// {
//     // Attach rising and falling edge on cdPin
//     // When card inserted, cdPin goes LOW; when removed, goes HIGH (assuming INPUT_PULLUP wiring)
//     attachInterrupt(cdPin, SDCard::cardDetectISR, CHANGE);
// }

// Called periodically in non-ISR context to handle the event
void SDCard::handleCardDetectEvent()
{
    if (!_pendingEvent)
        return;

    // Clear the flag
    _pendingEvent = false;

    if (_cardPresent)
    {
        // Card was just inserted
        Serial.println("SD: Card Inserted → initializing...");
        if (init())
        {
            Serial.println("SD: Initialization succeeded");
        }
        else
        {
            Serial.println("SD: Initialization failed");
        }
    }
    else
    {
        // Card was just removed
        Serial.println("SD: Card Removed → deinitializing...");
        // If you want to clean up, close any open files:
        // sd.closeAll();
        initialized = false;
    }
}
