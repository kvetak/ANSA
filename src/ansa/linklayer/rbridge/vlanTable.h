
/*
 -- TODO B1 --
 - VLAN ID
 - VLAN ifaces mapping


*/



#ifndef __VLANTABLE_H__
#define __VLANTABLE_H__

#include "inet/common/INETDefs.h"

//#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
//#include "IPvXAddressResolver.h"
//#include "inet/networklayer/contract/IRoutingTable.h"
//#include "RoutingTableAccess.h"
//#include "inet/networklayer/contract/IInterfaceTable.h"
//#include "InterfaceTableAccess.h"
//#include "IPv4Datagram.h"
//#include "TcpHeader.h"
//#include "UDPPacket.h"
//#include "NotificationBoard.h"
//#include "inet/linklayer/common/MacAddress.h"
//#include "inet/linklayer/ethernet/Ethernet.h"
//#include "inet/linklayer/ethernet/EtherFrame.h"


#include <string>
#include <vector>


#define VLANCOUNT 32

namespace inet {

class INET_API VLANTable : public cSimpleModule
{
public:

  VLANTable();
  ~VLANTable();


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

inline std::ostream& operator<<(std::ostream& os, const VLANTable::tTagAction a) {
	switch (a) {
	case VLANTable::INCLUDE:
		os << "T";
		break;
	case VLANTable::REMOVE:
		os << "U";
		break;
	case VLANTable::NONE:
		os << "??";
		break;
	}
	return os;
}


inline std::ostream& operator<<(std::ostream& os, const VLANTable::tVIDPortList l) {
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

inline std::ostream& operator<<(std::ostream& os, const VLANTable::tPortVIDRecord r) {
	os << "Port " << r.port << " accessing VLAN " << r.VID;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const VLANTable::tVIDRecord r) {
	os << "VLAN " << r.VID << " at ports " << r.portList;
	return os;
}


}
#endif

