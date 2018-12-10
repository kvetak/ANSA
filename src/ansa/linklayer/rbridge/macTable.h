/*
 * Author: Zdenek Kraus
 * email: xkraus00@stud.fit.vutbr.cz
 * created: 2010-02-12
 *
 */


/*
 -- TODO B1 --
 - VLAN ID
 - VLAN ifaces mapping


*/

#ifndef __MACTABLE_H__
#define __MACTABLE_H__


//#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MacAddress.h"

#include <string>
#include <vector>


namespace inet {

class MACTable : public cSimpleModule
{
public:
  MACTable();
  MACTable(int _tableSize);
  ~MACTable();


  typedef std::vector<int> tPortList;

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
  tSpec getSpec(MacAddress& addr);
  tPortList& getPorts(MacAddress& addr);
  void flush();
  void enableFasterAging(); // Aging ~ Ageing by Longman dictionary of contemporary english http://ldoceonline.com/
  void resetAging();


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
	AddressTable table;
	tPortList empty;


  int addressTableSize;       // Maximum size of the Address Table
  simtime_t agingTime;        // Determines when Ethernet entries are to be removed
  unsigned int uAgingTime;		  // user defined value (or default) for Stp aging reset
  simtime_t fasterAging;	  // for Stp topology change faster Aging

  virtual void initialize(); // TODO B2
  virtual void finish(); // TODO B2


};

inline std::ostream& operator<<(std::ostream& os, const MACTable::tPortList l) {
	os << "[";
	for (unsigned int i = 0; i < l.size(); i++) {
		if (i != 0) {
			os << ", ";
		}
		os << l.at(i);
	}
	os << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const MACTable::tType t) {
	switch (t) {
	case MACTable::STATIC:
		os << "Static";
		break;
	case MACTable::DYNAMIC:
		os << "Dynamic";
		break;
	case MACTable::GROUP:
		os << "Group";
		break;
	}

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const MACTable::tSpec s) {
	switch (s) {
	case MACTable::NONE:
		os << "None";
		break;
	case MACTable::Stp:
		os << "Stp";
		break;
	default:
		os << "???";
		break;
	}

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const MACTable::tRecord& addr) {
	os << addr.type;
	if (addr.spec != MACTable::NONE) {
		os << "(" << addr.spec << ")";
	}
	os << " " << addr.portList << " @" << addr.insert_time;
	return os;
}


}

#endif

