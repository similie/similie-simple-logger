#include "storage.h"

PayloadStore::PayloadStore()
{
}

bool PayloadStore::push(String topic, String payload)
{
    if (!Storage.sdCardPresent())
    {
        return false;
    }
    uint64_t addPosition = Storage.appendln(storeFile, sanitize(topic + "|" + payload));
    return addPosition > 0;
    // return Storage.appendlnAsync(storeFile, sanitize(topic + "|" + payload));
}

void PayloadStore::resetStorageFile()
{
    if (!Storage.sdCardPresent())
    {
        return;
    }
    setPopPosition(0);
    Storage.overwrite(storeFile.c_str(), "");
}

String *PayloadStore::pop(uint8_t size)
{
    String *result = new String[size];
    unsigned long position = getPopPosition();

    Serial.println("Pop position: " + String(position));
    for (uint8_t i = 0; i < size; i++)
    {
        result[i] = "";
        if (!Storage.sdCardPresent())
        {
            continue;
        }

        String value = Storage.read(storeFile, position, '\n');
        // here we have an empty value, we can break the loop.
        if (position == 0 && value.equals(""))
        {
            break;
            // we are at the end of file, we need to reset i
        }
        else if (value.equals(""))
        {
            position = 0;
            resetStorageFile();
            break;
        }

        result[i] = value;
    }
    Serial.println("Pop result POSITION: " + String(position));
    if (position > 0)
    {
        setPopPosition(position);
    }

    return result;
}

unsigned long PayloadStore::getPopPosition()
{
    if (!Storage.sdCardPresent())
    {
        return 0;
    }

    unsigned long position = 0;

    return position;
    String value = Storage.read(positionFile, position, '\n');

    if (value.equals(""))
    {
        setPopPosition(0);
        return 0;
    }
    return value.toInt();
}

bool PayloadStore::setPopPosition(unsigned long position)
{
    if (!Storage.sdCardPresent())
    {
        return false;
    }
    String value = String(position) + "\n";
    Storage.overwrite(positionFile.c_str(), value.c_str());
    return true;
}

String PayloadStore::setStale(String payload)
{
    JSONValue root = JSONValue::parseCopy(payload.c_str());
    if (!root.isValid() || !root.isObject())
    {
        return payload;
    }

    // Allocate output buffer (orig length + overhead)
    size_t outSize = payload.length() + 32;
    char outBuf[outSize];
    memset(outBuf, 0, outSize);

    JSONBufferWriter writer(outBuf, outSize - 1);
    writer.beginObject();

    // Copy existing fields
    JSONObjectIterator iter(root);
    while (iter.next())
    {
        // Use .data() to get a const char*
        const char *key = iter.name().data();
        writer.name(key);

        JSONValue v = iter.value();
        switch (v.type())
        {
        case JSON_TYPE_BOOL:
            writer.value(v.toBool());
            break;
        case JSON_TYPE_NUMBER:
            writer.value(v.toDouble());
            break;
        case JSON_TYPE_STRING:
            // toString() returns JSONString, use .data()
            writer.value(v.toString().data());
            break;
        default:
            // Fallback: stringify any other type
            writer.value(v.toString().data());
        }
    }

    // Append the stale flag
    writer.name("stale").value(true);

    writer.endObject();
    // Null-terminate safely
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;

    return String(outBuf);
}

void PayloadStore::addBackOntoStore(uint8_t startIndex, String *result, uint8_t size)
{

    for (uint8_t i = startIndex; i < size; i++)
    {
        if (result[i].equals(""))
        {
            break;
        }
        Serial.printf("Adding back onto store: %s\n", result[i].c_str());
        uint32_t addPosition = Storage.appendln(storeFile, sanitize(result[i]));
        if (addPosition == 0)
        {
            break;
        }
    }
}

uint32_t PayloadStore::countEntries()
{
    // No SD, no entries
    if (!Storage.sdCardPresent())
    {
        return 0;
    }

    uint32_t count = 0;
    unsigned long pos = getPopPosition();
    // Read until we hit an empty record
    while (true)
    {
        // grab everything from pos up to the next '\n'
        String line = Storage.read(storeFile, pos, '\n');
        if (line.equals(""))
        {
            // either EOF or blank line → stop
            break;
        }
        count++;
    }

    return count;
}

uint8_t PayloadStore::popOfflineCollection(uint8_t size, unsigned long delayTime)
{
    String *result = pop(size);
    uint8_t count = 0;
    for (uint8_t i = 0; i < size; i++)
    {
        delay(delayTime);
        if (result[i].equals(""))
        {
            break;
        }

        String topic = result[i].substring(0, result[i].indexOf("|"));
        String payload = result[i].substring(result[i].indexOf("|") + 1);
        String send = setStale(payload);
        if (send.equals(""))
        {
            continue;
        }
        Serial.printf("Topic: %s \n", topic.c_str());
        Serial.println("Sending offline payload: " + send);
        if (Particle.publish(sanitize(topic), setStale(payload)))
        {
            Serial.println("Offline payload sent successfully");
            count++;
        }
        else
        {
            Serial.println("Failed to send offline payload");
            break;
        }
    }
    Serial.println("Count: " + String(count) + " / " + String(size));
    if (count < size)
    {
        addBackOntoStore(count, result, size);
    }

    delete[] result; // Deallocate the array
    return count;
}

uint8_t PayloadStore::popOneOffline()
{
    return popOfflineCollection(1, 10);
}

uint8_t PayloadStore::popOfflineCollection()
{
    return popOfflineCollection(MAX_PAYLOADS, 1000);
}