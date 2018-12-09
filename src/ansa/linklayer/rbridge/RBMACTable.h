// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

/**
 * @file RBMACTable.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 19.3.2013
 * @brief Represents forwarding table. Based on Zdenek Kraus's MACTable.
 * @detail Represents forwarding table. Based on Zdenek Kraus's MACTable.
 * @todo Z9
 */

#ifndef RBMACTABLE_H_
#define RBMACTABLE_H_


#include "inet/common/INETDefs.h"
#include <string>
#include <vector>
#include <map>
#include <utility>


#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/common/MacAddress.h"

#include "ansa/linklayer/rbridge/macTable.h"
#include "ansa/linklayer/rbridge/TRILLCommon.h"

namespace inet {

class INET_API RBMACTable : public cSimpleModule
{
public:
  RBMACTable();
  RBMACTable(int _tableSize);
  ~RBMACTable();




  typedef std::vector<int> tPortList;

  typedef std::pair<MacAddress, int> ESTKey;

  typedef enum {
      EST_EMPTY, //empty record
      EST_LOCAL_PROCESS, // local processing
      EST_LOCAL_PORT, // for unicast destination (unicast source during learning)
      EST_RBRIDGE, // send to RBRidge
      EST_MULTICAST, //this option will be specified later
  } ESTRecordType;

  //Specify inserting method
  typedef enum  {
      //End Station(system) Record
      ESR_STATIC, // standart values and inserted by MGMT
      ESR_DYNAMIC, // inserted by learning process, aged out
      ESR_GROUP, // inserted by Group MGMT process
  } ESTInputType;

  typedef struct  {
          std::vector<int> portList; //vector is inteded for group addresses (multicast)
          SystemID systemID; //Use ingressNickname instead  ... //Egress RBridge System ID
          TRILLNickname ingressNickname; //ingressNickname
          unsigned int confidence; //TODO A1 (might be useful RFC 6325 section 4.8.1 at the very end)
          ESTRecordType type; //how to handle record type
          ESTInputType inputType; //specify inserting method
          SimTime insertTime;
          //little bit of redundancy
          MacAddress address;
          int vlanId;
  }ESTRecord;

  /* compare structure for std::map */
  struct ESRecordCompare{
          bool operator()(const ESTKey& u1, const ESTKey& u2) const{
              if (u1.second < u2.second)
              {
                  return true;
              }
              else if (u1.second == u2.second)
              {
                  if (u1.first.compareTo(u2.first) > 0)
                  {
                      return true;
                  }
                  else
                  {
                      return false;
                  }
              }
              else
              {
                  return false;
              }
         }
     };
  typedef std::map<ESTKey, ESTRecord, ESRecordCompare> ESTable; //end station table

  /* record types */
  typedef enum e_type {
      STATIC = 0, // standart values and inserted by MGMT
      DYNAMIC = 1, // inserted by learning process, aged out
      GROUP = 2, // inserted by Group MGMT process
  } tType;

  /* special definition */
  typedef enum e_spec {
      NONE = 0,
      Stp = 1, // switching to Stp ports
  } tSpec;

  /* enahanced MAC table record */
  typedef struct s_record {
      MacAddress addr; // mac address
      simtime_t insert_time; // time of insertion of update for ageing process
      tPortList portList; // list of destination ports (multiple ports for group adresses)
      tType type; // record type = {static, dynamic, group}
      tSpec spec; // address specialities
  } tRecord;

  /* compare structure for std::map */
  struct MacCompare{
      bool operator()(const MacAddress& u1, const MacAddress& u2) const
          {return u1.compareTo(u2) < 0;}
  };

  /* table map type */
  typedef std::map<MacAddress, tRecord, MacCompare> AddressTable;

  /* PUBLIC METHODS */
  void update(MacAddress& addr, int port);
  void updateNative(MacAddress& addr, int vlanId, int gateId);
  void updateTRILLData(MacAddress& addr, int vlanId, TRILLNickname ingressNickname);
  tSpec getSpec(MacAddress& addr);
  tPortList& getPorts(MacAddress& addr);
  void flush();
  void enableFasterAging(); // Aging ~ Ageing by Longman dictionary of contemporary english http://ldoceonline.com/
  void resetAging();

  ESTRecord getESTRecordByESTKey(ESTKey eSTKey);

  const AddressTable * getTable();

private:
  void initDefaults();

  /* MGMT */
  void flushAged();
  void removeOldest();
  void add(MacAddress addr, int port, tType type, tSpec spec);
  void remove(MacAddress addr);
  void removePort(MacAddress addr, int port);
  void addStatic(MacAddress addr, tPortList ports);

  /* mCast */
  void addGroup(MacAddress addr, tPortList ports); // TODO B2
  void addGroupPort(MacAddress addr, int port); // TODO B2
  void removeGroup(MacAddress addr); // TODO B2
  void removeGroupPort(MacAddress addr, int port); // TODO B2
  void alterGroup(MacAddress addr, tPortList ports); // TODO B2

protected:
  ESTable eSTable; //end station table
    AddressTable table;
    tPortList empty;
    ESTRecord emptyESRecord;


  int addressTableSize;       // Maximum size of the Address Table
  simtime_t agingTime;        // Determines when Ethernet entries are to be removed
  unsigned int uAgingTime;        // user defined value (or default) for Stp aging reset
  simtime_t fasterAging;      // for Stp topology change faster Aging

  virtual void initialize(); // TODO B2
  virtual void finish(); // TODO B2


};

//inline std::ostream& operator<<(std::ostream& os, const RBMACTable::tPortList l) {
//    os << "[";
//    for (unsigned int i = 0; i < l.size(); i++) {
//        if (i != 0) {
//            os << ", ";
//        }
//        os << l.at(i);
//    }
//    os << "]";
//    return os;
//}

inline std::ostream& operator<<(std::ostream& os, const RBMACTable::ESTKey key){
    os <<"MAC address: " << key.first << " and VLAN: " << key.second;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const RBMACTable::ESTRecordType t) {
    switch (t) {
        case RBMACTable::EST_EMPTY:
            os << "Empty";
            break;
        case RBMACTable::EST_LOCAL_PROCESS:
            os << "Local processing";
                        break;
        case RBMACTable::EST_LOCAL_PORT: // for unicast destination (unicast source during learning)
            os << "Local port";
            break;
        case RBMACTable::EST_RBRIDGE: // send to RBRidge
            os << "Remote RBridge";
            break;
        case RBMACTable::EST_MULTICAST: //this option will be specified later
            os << "Multi-destination";
            break;
    }

    return os;
}

//inline std::ostream& operator<<(std::ostream& os, const std::vector<int> t) {
//
//
//  for(auto it = t.begin(); it != t.end(); it++){
//    os << (*it) << ", ";
//  }
//
//  return os;
//}


inline std::ostream& operator<<(std::ostream& os, const RBMACTable::ESTRecord record){
//    os <<"MAC address: " << key.first << " and VLAN: " << key.second;


//    os << addr.type;
//    if (addr.spec != RBMACTable::NONE) {
//        os << "(" << addr.spec << ")";
//    }
//    os << " " << addr.portList << " @" << addr.insert_time;
//    return os;

    os << record.type;
    os << " " << record.portList << " @" << record.insertTime;
    return os;
}



inline std::ostream& operator<<(std::ostream& os, const RBMACTable::tType t) {
    switch (t) {
    case RBMACTable::STATIC:
        os << "Static";
        break;
    case RBMACTable::DYNAMIC:
        os << "Dynamic";
        break;
    case RBMACTable::GROUP:
        os << "Group";
        break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const RBMACTable::tSpec s) {
    switch (s) {
    case RBMACTable::NONE:
        os << "None";
        break;
    case RBMACTable::Stp:
        os << "Stp";
        break;
    default:
        os << "???";
        break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const RBMACTable::tRecord& addr) {
    os << addr.type;
    if (addr.spec != RBMACTable::NONE) {
        os << "(" << addr.spec << ")";
    }
    os << " " << addr.portList << " @" << addr.insert_time;
    return os;
}

}
#endif /* RBMACTABLE_H_ */
