#include "Particle.h"
#include "string.h"

#ifndef heartbeat_h
#define heartbeat_h

#define HAS_LOCAL_POWER false

class HeartBeat
{
private:
    String deviceID;
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