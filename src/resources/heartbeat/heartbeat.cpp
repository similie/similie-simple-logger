#include "heartbeat.h"
#include "CellularHelper.h"

HeartBeat::~HeartBeat()
{
}

HeartBeat::HeartBeat(String deviceID)
{
    this->deviceID = deviceID;
}

String HeartBeat::cellAccessTech(int rat)
{
    switch (rat)
    {
    case NET_ACCESS_TECHNOLOGY_GSM:
        return String("2G RAT");
    case NET_ACCESS_TECHNOLOGY_EDGE:
        return String("2G RAT with EDGE");
    case NET_ACCESS_TECHNOLOGY_UMTS:
        return String("UMTS RAT");
    case NET_ACCESS_TECHNOLOGY_LTE:
        return String("LTE RAT");
    }

    return String("ACCESS UNDEFINED");
}

void HeartBeat::setSystemDeets(JSONBufferWriter &writer)
{
    writer.name("system").beginObject();
    int freemem = (uint32_t)System.freeMemory();
    writer.name("freemem").value(freemem);
    int uptime = System.uptime();
    writer.name("uptime").value(uptime);
    writer.endObject();
}
void HeartBeat::setPowerlDeets(JSONBufferWriter &writer)
{
    writer.name("power").beginObject();
    float vCel = fuel.getVCell();
    writer.name("v_cel").value(vCel);
    float soc = fuel.getSoC();
    writer.name("SoC").value(soc);
    float bat = System.batteryCharge();
    writer.name("bat").value(bat);
    if (HAS_LOCAL_POWER)
    {
        // @todo:: pull some boomo board power
    }
    writer.endObject();
}

void HeartBeat::setCellDeets(JSONBufferWriter &writer)
{
    CellularSignal sig = Cellular.RSSI();
    writer.name("cellular").beginObject();
    int rat = sig.getAccessTechnology();
    String tech = this->cellAccessTech(rat);
    writer.name("tech").value(tech.c_str());
    float strength = sig.getStrength();
    writer.name("strength").value(strength);
    float quality = sig.getQuality();
    writer.name("quality").value(quality);
    float strengthVal = sig.getStrengthValue();
    writer.name("strength_val").value(strengthVal);
    float qualityVal = sig.getQualityValue();
    writer.name("quality_val").value(qualityVal);
    String ip = Cellular.localIP().toString();
    writer.name("local_ip").value(ip);
    String sim = CellularHelper.getICCID();
    writer.name("ICCID").value(sim);
    String carrier = CellularHelper.getOperatorName();
    writer.name("carrier").value(carrier);
    String imei = CellularHelper.getIMEI();
    writer.name("IMEI").value(imei);
    writer.endObject();
}

String HeartBeat::pump()
{
    char buf[800];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    writer.name("device").value(this->deviceID);
    writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
    setCellDeets(writer);
    setPowerlDeets(writer);
    setSystemDeets(writer);
    writer.endObject();
    String pump = String(buf);
    return pump;
}