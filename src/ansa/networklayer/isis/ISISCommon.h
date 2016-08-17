

#ifndef ISISCOMMON_H_
#define ISISCOMMON_H_


#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"


namespace inet{
static const int ISIS_SYSTEM_ID = 6;
static const int SYSTEMID_STRING_SIZE = (ISIS_SYSTEM_ID * 2) + 2; //printed in hexa + two dots 0000.0000.0000
static const int LANID_STRING_SIZE = 3; //SYSTEM_ID + dot and NSEL (2 chars)
static const int ISIS_LAN_ID_TLV_LEN = ISIS_SYSTEM_ID + 1;
static const int ISIS_AREA_ID_LEN = 3;
static const int ISIS_AREA_ADDRESS_TLV_LEN = ISIS_AREA_ID_LEN + 1;
static const int ISIS_LSP_ID_TLV_LEN = ISIS_SYSTEM_ID + 2;

class SystemID {

protected:
    uint64 systemID;
public:

    SystemID(){
      systemID = 0;
    }
    SystemID(uint64 sysID){
      systemID = sysID;
    }
    SystemID(const SystemID& sysID){
      systemID = sysID.getSystemId();
    }
    uint64 getSystemId() const {
        return systemID;
    }

    void setSystemId(uint64 systemId) {
        systemID = systemId;
    }

    virtual SystemID toSysID()const{
      return SystemID(systemID);
    }

    virtual uint64 toInt() const {
      return systemID;
    }

    virtual std::string str() const
    {

        char buf[SYSTEMID_STRING_SIZE];
        sprintf(buf, "%04X.%04X.%04X", (systemID >> 32) & (0xFFFF), (systemID >> 16) & (255*255), systemID & (255*255));
        return std::string(buf);
    }

    bool operator==(const SystemID& sysID) const {return systemID == sysID.getSystemId();}
    bool operator!=(const SystemID& sysID) const {return systemID != sysID.getSystemId();}

    bool operator<(const SystemID& sysID) const {return systemID < sysID.getSystemId();}
    bool operator<=(const SystemID& sysID) const {return systemID <= sysID.getSystemId();}

    bool operator>(const SystemID& sysID) const {return systemID > sysID.getSystemId();}
    bool operator>=(const SystemID& sysID) const {return systemID >= sysID.getSystemId();}



};


inline std::ostream& operator<<(std::ostream& os, const SystemID& sysID)
{
    return os << sysID.str();
}

class AreaID {


private:
    uint64 areaID;
public:

    AreaID(){
        areaID = 0;
    }

    uint64 getAreaId() const {
        return areaID;
    }

    void setAreaId(uint64 areaId) {
        areaID = areaId;
    }

    uint64 toInt(){
        return areaID;
    }

    /**
     * This method expects areaID in the form of 3B unsigned char
     */
    void setAreaId(const unsigned char * areaId){

        areaID = strtoul((const char *)areaId, NULL, 16);

    }

    unsigned char *toTLV() const{
        unsigned char *buffer = new unsigned char[ISIS_AREA_ADDRESS_TLV_LEN];
        buffer[0] = ISIS_AREA_ID_LEN & 0xFF;
        buffer[1] = (areaID ) & 0xFF;
        buffer[2] = (areaID >> 8) & 0xFF;
        buffer[3] = (areaID >> 16) & 0xFF;
        return buffer;
    }

    void fromTLV(const unsigned char * areaIDTLV)
    {
        std::string helper(reinterpret_cast<const char *>(areaIDTLV));
        areaID = strtoul(helper.substr(1, 3).c_str(), NULL, 16);

    }

    unsigned char *toChar() const{
        unsigned char *buffer = new unsigned char[ISIS_AREA_ID_LEN];
        buffer[0] = (areaID ) & 0xFF;
        buffer[1] = (areaID >> 8) & 0xFF;
        buffer[2] = (areaID >> 16) & 0xFF;
        return buffer;
    }

    virtual std::string str() const
    {

        char buf[SYSTEMID_STRING_SIZE];
        sprintf(buf, "%02X.%04X", (areaID >> 16) & (0xFF), areaID & (255*255));
        return std::string(buf);
    }


    bool operator==(const AreaID& other) const {return areaID == other.getAreaId();}
    bool operator!=(const AreaID& other) const {return areaID != other.getAreaId();}

    bool operator<(const AreaID& other) const {return areaID < other.getAreaId();}
    bool operator<=(const AreaID& other) const {return areaID <= other.getAreaId();}

    bool operator>(const AreaID& other) const {return areaID > other.getAreaId();}
    bool operator>=(const AreaID& other) const {return areaID >= other.getAreaId();}

};


inline std::ostream& operator<<(std::ostream& os, const AreaID& areaID)
{
    return os << areaID.str();
}

class PseudonodeID : public SystemID
{
protected:
//    uint64 pseudoID;
//    SystemID systemID;
    uint circuitID;

public:


    PseudonodeID() {
//        pseudoID = 0;
      circuitID = 0;
    }
    PseudonodeID(uint64 sysID, uint circID){
//        pseudoID = systemID << 8;
//        pseudoID += circuitID & 255;

        systemID = sysID;
        circuitID = circID;
    }

    PseudonodeID(SystemID sysID, uint circID):SystemID(sysID){
//      pseudoID = systemID.getSystemId() << 8;
//      pseudoID += circuitID & 255;


//      systemID = sysID.getSystemId();
      circuitID = circID;
    }

    PseudonodeID(const PseudonodeID& pseudoID)
    {
      PseudonodeID(pseudoID.getSystemId(), pseudoID.getCircuitId());

    }

    void set(SystemID sysID, uint circID){
      systemID = sysID.getSystemId();
      circuitID = circID;
    }
    uint getCircuitId() const
    {
      return circuitID;
    }

    void setCircuitId(uint circuitId)
    {
      circuitID = circuitId;
    }

    SystemID getSystemId() const
    {
      return SystemID(systemID);
    }

    void setSystemId(const SystemID systemId)
    {
      systemID = systemId.getSystemId();
    }

    unsigned char *toTLV() const{
        unsigned char *buffer = new unsigned char[ISIS_LAN_ID_TLV_LEN];

        for(int i = 0; i < ISIS_LAN_ID_TLV_LEN; i++){
            buffer[i] = (toInt() >> (i * 8)) & 0xFF;
        }

        return buffer;
    }

    void fromTLV(const unsigned char * lanIDTLV)
    {
        std::string helper(reinterpret_cast<const char *>(lanIDTLV));
        areaID = strtoul(helper.substr(1, ISIS_LAN_ID_TLV_LEN).c_str(), NULL, 16);

    }

    virtual uint64 toInt() const override{
      return (systemID << 8) + circuitID;
    }

    virtual std::string str() const
    {

        char buf[LANID_STRING_SIZE];
        sprintf(buf, ".%02X",  circuitID & (255));
        return std::string(SystemID::str() + buf);

    }


    bool operator==(const PseudonodeID& other) const {return toInt() == other.toInt();}
    bool operator!=(const PseudonodeID& other) const {return toInt() != other.toInt();}

    bool operator<(const PseudonodeID& other) const {return toInt() < other.toInt();}
    bool operator<=(const PseudonodeID& other) const {return toInt() <= other.toInt();}

    bool operator>(const PseudonodeID& other) const {return toInt() > other.toInt();}
    bool operator>=(const PseudonodeID& other) const {return toInt() >= other.toInt();}


};

inline std::ostream& operator<<(std::ostream& os, const PseudonodeID& pseudoID)
{
    return os << pseudoID.str();
}

class LspID : public PseudonodeID
{
protected:

    uint fragmentID;

public:

    LspID(){
        fragmentID = 0;
    }

    LspID(SystemID sysID):PseudonodeID(sysID, 0){
      LspID();
    }

    void set(const SystemID sysID){
      setSystemId(sysID);
      circuitID = 0;
      fragmentID = 0;
    }

    PseudonodeID getPseudonodeID()const{
      return PseudonodeID(getSystemId(),circuitID);
    }

    virtual uint64 toInt() const override {
      return (PseudonodeID::toInt() << 8) + circuitID;
    }

    void setMax() {
      systemID = UINT64_MAX;
      circuitID = UINT_MAX;
      fragmentID = UINT_MAX;
    }

    virtual std::string str() const
    {

        char buf[LANID_STRING_SIZE];
        sprintf(buf, "-%02X",  fragmentID & (255));
        return std::string(PseudonodeID::str() + buf);

    }

    unsigned char *toTLV() const{
        unsigned char *buffer = new unsigned char[ISIS_LSP_ID_TLV_LEN];

        for(int i = 0; i < ISIS_LSP_ID_TLV_LEN; i++){
            buffer[i] = (toInt() >> (i * 8)) & 0xFF;
        }
//        buffer[0] = toInt() & 0xFF;
//        buffer[1] = (toInt() >> 8) & 0xFF;
//        buffer[2] = (toInt >> 8) & 0xFF;
//        buffer[3] = (toInt >> 16) & 0xFF;
//        buffer[4] = (toInt >> 16) & 0xFF;
//        buffer[5] = (toInt >> 16) & 0xFF;
//        buffer[6] = (toInt >> 16) & 0xFF;
//        buffer[7] = (toInt >> 16) & 0xFF;
        return buffer;
    }

    void fromTLV(const unsigned char * areaIDTLV)
    {
        std::string helper(reinterpret_cast<const char *>(areaIDTLV));
        systemID = strtoul(helper.substr(0, ISIS_SYSTEM_ID).c_str(), NULL, 16);
        circuitID = strtoul(helper.substr(ISIS_SYSTEM_ID, 1).c_str(), NULL, 16);
        fragmentID = strtoul(helper.substr(ISIS_SYSTEM_ID + 1, 1).c_str(), NULL, 16);

    }


    bool operator==(const LspID& other) const {return toInt() == other.toInt();}
    bool operator!=(const LspID& other) const {return toInt() != other.toInt();}

    bool operator<(const LspID& other) const {return toInt() < other.toInt();}
    bool operator<=(const LspID& other) const {return toInt() <= other.toInt();}

    bool operator>(const LspID& other) const {return toInt() > other.toInt();}
    bool operator>=(const LspID& other) const {return toInt() >= other.toInt();}

    uint getFragmentId() const
    {
      return fragmentID;
    }

    void setFragmentId(uint fragmentId)
    {
      fragmentID = fragmentId;
    }
};

inline std::ostream& operator<<(std::ostream& os, const LspID& lspID)
{
    return os << lspID.str();
}

}//end namespace


#endif /* ISISCOMMON_H_ */
