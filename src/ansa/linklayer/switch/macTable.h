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


#include "linklayer/common/MACAddress.h"
#include "linklayer/ethernet/Ethernet.h"
#include "linklayer/ethernet/EtherFrame_m.h"

#include <string>
#include <vector>


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
	  STP = 1, // switching to STP ports
  } tSpec;

  /* enahanced MAC table record */
  typedef struct s_record {
	  inet::MACAddress addr; // mac address
	  simtime_t insert_time; // time of insertion of update for ageing process
	  tPortList portList; // list of destination ports (multiple ports for group adresses)
	  tType type; // record type = {static, dynamic, group}
	  tSpec spec; // address specialities
  } tRecord;

  /* compare structure for std::map */
  struct MAC_compare{
	  bool operator()(const inet::MACAddress& u1, const inet::MACAddress& u2) const
		  {return u1.compareTo(u2) < 0;}
  };

  /* table map type */
  typedef std::map<inet::MACAddress, tRecord, MAC_compare> AddressTable;

  /* PUBLIC METHODS */
  void update(inet::MACAddress& addr, int port);
  tSpec getSpec(inet::MACAddress& addr);
  tPortList& getPorts(inet::MACAddress& addr);
  void flush();
  void enableFasterAging(); // Aging ~ Ageing by Longman dictionary of contemporary english http://ldoceonline.com/
  void resetAging();


  const AddressTable * getTable();

private:
  void initDefaults();

  /* MGMT */
  void flushAged();
  void removeOldest();
  void add(inet::MACAddress addr, int port, tType type, tSpec spec);
  void remove(inet::MACAddress addr);
  void removePort(inet::MACAddress addr, int port);
  void addStatic(inet::MACAddress addr, tPortList ports);

  /* mCast */
  void addGroup(inet::MACAddress addr, tPortList ports); // TODO B2
  void addGroupPort(inet::MACAddress addr, int port); // TODO B2
  void removeGroup(inet::MACAddress addr); // TODO B2
  void removeGroupPort(inet::MACAddress addr, int port); // TODO B2
  void alterGroup(inet::MACAddress addr, tPortList ports); // TODO B2

protected:
	AddressTable table;
	tPortList empty;


  int addressTableSize;       // Maximum size of the Address Table
  simtime_t agingTime;        // Determines when Ethernet entries are to be removed
  unsigned int uAgingTime;		  // user defined value (or default) for STP aging reset
  simtime_t fasterAging;	  // for STP topology change faster Aging

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
	case MACTable::STP:
		os << "STP";
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

#endif

