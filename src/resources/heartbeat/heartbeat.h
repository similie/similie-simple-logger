#include "Particle.h"
#include "string.h"
#include "resources/bootstrap/buffer-manager.h"
#ifndef heartbeat_h
#define heartbeat_h

#define HAS_LOCAL_POWER false
#define HEART_BUFFER_SIZE 400

class HeartBeat
{
private:
    // char buf[HEART_BUFFER_SIZE];
    String deviceID;
    FuelGauge fuel;
    String cellAccessTech(int rat);
    void setCellDeets(JSONBufferWriter &writer);
    void setPowerDeets(JSONBufferWriter &writer);
    void setSystemDeets(JSONBufferWriter &writer);

public:
    ~HeartBeat();
    HeartBeat(String deviceID);
    String pump();
};

#endif