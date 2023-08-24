#include "Particle.h"
#include "string.h"
#define HAS_LOCAL_POWER false
#define BUFFERSIZE 800
#ifndef heartbeat_h
#define heartbeat_h
class HeartBeat
{
private:
    char buf[BUFFERSIZE];
    String deviceID;
    FuelGauge fuel;
    String cellAccessTech(int rat);
    void setCellDeets(JSONBufferWriter &writer);
    void setPowerlDeets(JSONBufferWriter &writer);
    void setSystemDeets(JSONBufferWriter &writer);

public:
    ~HeartBeat();
    HeartBeat(String deviceID);
    String pump();
};

#endif