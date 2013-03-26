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
 * @file RBVLANTable.h
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 20.3.2013
 * @brief
 * @detail
 * @todo
 */

#ifndef RBVLANTABLE_H_
#define RBVLANTABLE_H_

#include <csimplemodule.h>

#include "IPv4Address.h"
#include "IPvXAddressResolver.h"
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPv4Datagram.h"
#include "TCPSegment.h"
#include "UDPPacket.h"
#include "NotificationBoard.h"
#include "MACAddress.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"


#include <string>
#include <vector>


#define VLANCOUNT 32

class INET_API RBVLANTable : public cSimpleModule
{
public:

  RBVLANTable();
  ~RBVLANTable();


  /* VLAN -> Port mapping */
  typedef enum e_tag_action {
      REMOVE = 0,
      INCLUDE = 1,
      NONE = -1,
  } tTagAction;

  typedef struct s_vid_port {
      int port;
      tTagAction action;
  } tVIDPort;
  /* Vector of port->action pairs */
  typedef std::vector<tVIDPort> tVIDPortList;

  /* VLAN struct, defines VLAN ID, name and associated ports */
  typedef struct s_vid_record {
      int VID;
      std::string name;
      tVIDPortList portList; /* Vector of port->action pairs */
  } tVIDRecord;

  /* Port -> VLAN mapping */
  typedef struct s_port_vid {
      int port;
      int VID;
  } tPortVIDRecord;


  typedef std::vector<tVIDRecord> VIDTable;
  /* Port -> VLAN mapping */
  typedef std::vector<tPortVIDRecord> PortVIDTable;

  /* MGMT */
  const VIDTable * getTaggedTable();
  const PortVIDTable * getUntaggedTable();

  /* PUBLIC ACCESS METHODS */
  tVIDPortList getPorts(int VID);
  int getVID(int Port);
  bool isAllowed(int VID, int _port);
  tTagAction getTag(int VID, int _port);

  /* MGMT */
    void add(int VID, tVIDPortList& _portList);
    void addTagged(int VID, std::vector<int>& ports);
    void addUntagged(int VID, std::vector<int>& ports);

    void setVLANName(int, std::string&);

    void addPortVID(int _port, int _VID);
    void setPortVID(int _port, int _VID);

    void delPort(int _port, int _VID);
    /* Register specified VLAN */
    void regVLAN(unsigned int vlanID);
    std::vector<unsigned int> getVLANList();

    void initDefault();

    /* add empty records at the end of existing until given VLAN, inclusive*/
    void extendTable(int VLAN);

private:
    VIDTable vidTable;
    /* Table (vector) of port -> VID(VLAN-ID) pairs*/
    PortVIDTable portVIDTable;
    tVIDPortList empty;

    tVIDRecord emptyVID;

    /* Vector of registered VLAN-IDs */
    std::vector<unsigned int> vlanList;

    int portCount;


protected:

  virtual void initialize(int stage);
  virtual int numInitStages() const {return 1;}
  virtual void finish();



};

inline std::ostream& operator<<(std::ostream& os, const RBVLANTable::tTagAction a) {
    switch (a) {
    case RBVLANTable::INCLUDE:
        os << "T";
        break;
    case RBVLANTable::REMOVE:
        os << "U";
        break;
    case RBVLANTable::NONE:
        os << "??";
        break;
    }
    return os;
}


inline std::ostream& operator<<(std::ostream& os, const RBVLANTable::tVIDPortList l) {
    os << "[";
    for (unsigned int i = 0; i < l.size(); i++) {
        if (i != 0) {
            os << ", ";
        }
        os << l.at(i).port << l.at(i).action;
    }
    os << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const RBVLANTable::tPortVIDRecord r) {
    os << "Port " << r.port << " accessing VLAN " << r.VID;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const RBVLANTable::tVIDRecord r) {
    os << "VLAN " << r.VID << " at ports " << r.portList;
    return os;
}



#endif /* RBVLANTABLE_H_ */
