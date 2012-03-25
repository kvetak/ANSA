/*
 * Author: Zdenek Kraus
 * email: xkraus00@stud.fit.vutbr.cz
 * created: 2010-02-12
 *
 */


/*
 -- TODO --
 - VLAN ID
 - VLAN ifaces mapping


*/

#ifndef __MACTABLE_H__
#define __MACTABLE_H__


#include "MACAddress.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"

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
	  MACAddress addr; // mac address
	  simtime_t insert_time; // time of insertion od update for ageing process
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
  tSpec getSpec(MACAddress& addr);
  tPortList& getPorts(MACAddress& addr);
  void flush();
  void enableFasterAging(); // Aging ~ Ageing by Longman dictionary of contemporary english http://ldoceonline.com/
  void resetAging();


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
	AddressTable table;
	tPortList empty;


  int addressTableSize;       // Maximum size of the Address Table
  simtime_t agingTime;        // Determines when Ethernet entries are to be removed
  unsigned int uAgingTime;		  // user defined value (or default) for STP aging reset
  simtime_t fasterAging;	  // for STP topology change faster Aging

  virtual void initialize(); // TODO
  virtual void finish(); // TODO


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

