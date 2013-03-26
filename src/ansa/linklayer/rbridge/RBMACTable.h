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
 * @brief
 * @detail
 * @todo
 */

#ifndef RBMACTABLE_H_
#define RBMACTABLE_H_




#include "MACAddress.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "macTable.h"
#include <string>
#include <vector>
#include <map>
#include <utility>


class RBMACTable : public cSimpleModule
{
public:
  RBMACTable();
  RBMACTable(int _tableSize);
  ~RBMACTable();




  typedef std::vector<int> tPortList;

  typedef std::pair<MACAddress, int> ESTKey;

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
          unsigned char systemID[6]; //Egress RBridge System ID
          ESTRecordType type; //how to handle record type
          ESTInputType inputType; //specify inserting method
          SimTime insertTime;
          //little bit of redundancy
          MACAddress address;
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
      STP = 1, // switching to STP ports
  } tSpec;

  /* enahanced MAC table record */
  typedef struct s_record {
      MACAddress addr; // mac address
      simtime_t insert_time; // time of insertion of update for ageing process
      tPortList portList; // list of destination ports (multiple ports for group adresses)
      tType type; // record type = {static, dynamic, group}
      tSpec spec; // address specialities
  } tRecord;

  /* compare structure for std::map */
  struct MAC_compare{
      bool operator()(const MACAddress& u1, const MACAddress& u2) const
          {return u1.compareTo(u2) < 0;}
  };

  /* table map type */
  typedef std::map<MACAddress, tRecord, MAC_compare> AddressTable;

  /* PUBLIC METHODS */
  void update(MACAddress& addr, int port);
  void update(MACAddress& addr, int vlanId, int gateId);
  tSpec getSpec(MACAddress& addr);
  tPortList& getPorts(MACAddress& addr);
  void flush();
  void enableFasterAging(); // Aging ~ Ageing by Longman dictionary of contemporary english http://ldoceonline.com/
  void resetAging();

  ESTRecord& getESTRecordByESTKey(ESTKey eSTKey);

  const AddressTable * getTable();

private:
  void initDefaults();

  /* MGMT */
  void flushAged();
  void removeOldest();
  void add(MACAddress addr, int port, tType type, tSpec spec);
  void remove(MACAddress addr);
  void removePort(MACAddress addr, int port);
  void addStatic(MACAddress addr, tPortList ports);

  /* mCast */
  void addGroup(MACAddress addr, tPortList ports); // TODO
  void addGroupPort(MACAddress addr, int port); // TODO
  void removeGroup(MACAddress addr); // TODO
  void removeGroupPort(MACAddress addr, int port); // TODO
  void alterGroup(MACAddress addr, tPortList ports); // TODO

protected:
  ESTable eSTable; //end station table
    AddressTable table;
    tPortList empty;
    ESTRecord emptyESRecord;


  int addressTableSize;       // Maximum size of the Address Table
  simtime_t agingTime;        // Determines when Ethernet entries are to be removed
  unsigned int uAgingTime;        // user defined value (or default) for STP aging reset
  simtime_t fasterAging;      // for STP topology change faster Aging

  virtual void initialize(); // TODO
  virtual void finish(); // TODO


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
    case RBMACTable::STP:
        os << "STP";
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


#endif /* RBMACTABLE_H_ */
