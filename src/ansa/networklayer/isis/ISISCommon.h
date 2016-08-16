

#ifndef ISISCOMMON_H_
#define ISISCOMMON_H_


#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"


namespace inet{
static const int ISIS_SYSTEM_ID = 6;
static const int SYSTEMID_STRING_SIZE = (ISIS_SYSTEM_ID * 2) + 2; //printed in hexa + two dots 0000.0000.0000
static const int LANID_STRING_SIZE = 3; //SYSTEM_ID + dot and NSEL (2 chars)

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
