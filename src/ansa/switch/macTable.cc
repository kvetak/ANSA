
#include "macTable.h"

Define_Module(MACTable);


/* --- PUBLIC --- */

MACTable::MACTable() {
  this->initDefaults();
  return;
}

MACTable::MACTable(int _tableSize) {
  this->initDefaults();
  if (_tableSize < 0) {
    this->addressTableSize = 0;
  } else {
    this->addressTableSize = _tableSize;
  }
  return;
}

MACTable::~MACTable() {
  return;
}

const MACTable::AddressTable * MACTable::getTable() {
  return &(this->table);
}



void MACTable::update(MACAddress& addr, int port) {
	Enter_Method_Silent();

    flushAged();

  AddressTable::iterator iter;

  iter = this->table.find(addr);
  if (iter == this->table.end()) {
    // Observe finite table size
    if (this->addressTableSize != 0 && this->table.size() == (unsigned int)addressTableSize) {
      EV << "Making room in Address Table by throwing out aged entries.\n";

      if (this->table.size() == (unsigned int)addressTableSize) {
        removeOldest();
      }
    }
    // Add entry to table
    EV << "Adding entry to Address Table: "<< addr << " --> port" << port << "\n";

    tRecord entry;
    entry.addr = addr;
    entry.insert_time = simTime();
    entry.portList.push_back(port);
    entry.type = DYNAMIC;
    entry.spec = NONE;

    this->table[addr] = entry;
  } else {
    // Update existing entry
    EV << "Updating entry in Address Table: "<< addr << " --> port" << port << "\n";

    tRecord& entry = iter->second;

    if (entry.type == DYNAMIC) {
		entry.insert_time = simTime();
		if (entry.portList.at(0) != port) {
			entry.portList.clear();
			entry.portList.push_back(port);
		}
    }
  }

  return;
}

MACTable::tSpec MACTable::getSpec(MACAddress& addr) {
	Enter_Method_Silent();

    AddressTable::iterator iter = table.find(addr);
    if (iter == table.end()) {
        // not found
        return NONE;
    }
    if ((iter->second.type != STATIC) &&
    	(iter->second.insert_time + agingTime) <= simTime()) {
        // don't use (and throw out) aged entries
        EV << "Ignoring and deleting aged entry: "<< iter->first << "\n";

        table.erase(iter);

        return NONE;
    }

    return iter->second.spec;
}


MACTable::tPortList& MACTable::getPorts(MACAddress& addr) {

	Enter_Method_Silent();

    AddressTable::iterator iter = table.find(addr);
    if (iter == table.end()) {
        // not found
        return empty;
    }
    if ((iter->second.type != STATIC) &&
    	(iter->second.insert_time + agingTime) <= simTime()) {
        // don't use (and throw out) aged entries
        EV << "Ignoring and deleting aged entry: "<< iter->first << "\n";

        table.erase(iter);

        return empty;
    }

    return iter->second.portList;
}

void MACTable::flush() {

	Enter_Method_Silent();

	AddressTable::iterator iter;
	for (iter = table.begin(); iter != table.end();) {
		AddressTable::iterator cur = iter++;
		tRecord entry = cur->second;
		if (entry.type == DYNAMIC) { //TODO GROUP adresses too ?
			table.erase(cur);
		}
	}

  return;
}

void MACTable::enableFasterAging() {
	Enter_Method_Silent();
	agingTime = fasterAging;
}

void MACTable::resetAging() {
	Enter_Method_Silent();
	agingTime = uAgingTime;
}


/* --- PRIVATE --- */


void MACTable::initDefaults() {
  addressTableSize = 1024; // TODO

  /* by IEEE 802.1D-1998
   * " The Bridges then use a short value
   * to age out dynamic entries
   * in the Fitering Database for a period."
   * */
  fasterAging = 5; // short value to age out ... (5s < 5xHelloTime = 10s)
  uAgingTime = 300; // recommended value by IEEE 802.1D-1998 (and later)...
  agingTime = uAgingTime; // renew of user defined value, is triggered by STP process and receiving bpdu
  return;
}

/* MGMT */

void MACTable::flushAged() {

	Enter_Method_Silent();

	AddressTable::iterator iter;
	for (iter = this->table.begin(); iter != this->table.end();) {
		AddressTable::iterator cur = iter++; // iter will get invalidated after erase()
		tRecord entry = cur->second;
		if (entry.type == DYNAMIC && entry.insert_time + agingTime <= simTime()) {
			EV << "Removing aged entry from Address Table: " << cur->first << "\n";
			this->table.erase(cur);
		}
	}
	return;
}

void MACTable::removeOldest() {

	Enter_Method_Silent();

	AddressTable::iterator oldest = this->table.end();
  simtime_t oldestInsertTime = simTime()+1;
  for (AddressTable::iterator iter = this->table.begin(); iter != this->table.end(); iter++) {
    if (iter->second.type != DYNAMIC && iter->second.insert_time < oldestInsertTime) {
      oldest = iter;
      oldestInsertTime = iter->second.insert_time;
    }
  }
  if (oldest != this->table.end()) {
    EV << "Table full, removing oldest entry: " <<
      oldest->first << "\n";
    this->table.erase(oldest);
  }
  return;
}


void MACTable::add(MACAddress addr, int port, tType type, tSpec spec) {
  AddressTable::iterator iter;

  iter = this->table.find(addr);
  if (iter == this->table.end()) { // record not found
	// Observe finite table size
	if (this->addressTableSize != 0 && this->table.size() == (unsigned int)addressTableSize) {
	  EV << "Making room in Address Table by throwing out aged entries.\n";

	  if (this->table.size() == (unsigned int)addressTableSize) {
		removeOldest();
	  }
	}
	// Add entry to table
	EV << "Adding entry to Address Table: "<< addr << " --> port" << port << "\n";

	tRecord entry; // create new entry, with given port & type
	entry.addr = addr;
	entry.insert_time = simTime();
	entry.portList.push_back(port);
	entry.type = type;
	entry.spec = spec;

	this->table[addr] = entry;
  } else {
	// Update existing entry
	EV << "Updating entry in Address Table: "<< addr << " --> port" << port << "\n";

	tRecord& entry = iter->second;

	entry.insert_time = simTime();
	unsigned int i;
	for (i = 0; i < entry.portList.size(); i++) { // search through portList for given port
		if (entry.portList.at(i) == port) {
			break;
		}
	}
	if (i < entry.portList.size()) {
		entry.portList.clear();
		entry.portList.push_back(port);
	}
  }

  return;
}

void MACTable::remove(MACAddress addr) {
	AddressTable::iterator iter;

	iter = this->table.find(addr);
	if (iter == this->table.end()) { // record not found
	  return;
	} else {
	  this->table.erase(iter);
	}
	return;
}

void MACTable::removePort(MACAddress addr, int port) {
	AddressTable::iterator iter;

	iter = this->table.find(addr);
	if (iter == this->table.end()) { // record not found
	  return;
	} else {
		tRecord& entry = iter->second;
		tPortList::iterator pIter;
		for (pIter = entry.portList.begin(); pIter < entry.portList.end(); pIter++) { // search through portList for given port
			if (*pIter == port) {
				entry.portList.erase(pIter);
				break;
			}
		}
	}
	return;
}

void MACTable::addStatic(MACAddress addr, tPortList ports) {
	AddressTable::iterator iter;

	iter = this->table.find(addr);
	if (iter == this->table.end()) { // record not found
	// Observe finite table size
	if (this->addressTableSize != 0 && this->table.size() == (unsigned int)addressTableSize) {
	  EV << "Making room in Address Table by throwing out aged entries.\n";

	  if (this->table.size() == (unsigned int)addressTableSize) {
		removeOldest();
	  }
	}
	// Add entry to table
	EV << "Adding static entry to Address Table: "<< addr << " " << ports << std::endl;

	tRecord entry; // create new entry, with given port & type
	entry.addr = addr;
	entry.insert_time = simTime();
	entry.portList = ports;
	entry.type = STATIC;
	entry.spec = NONE;

	this->table[addr] = entry;
	} else {
		// Update existing entry
		EV << "Updating entry in Address Table: "<< addr << " " << ports << std::endl;

		tRecord& entry = iter->second;

		entry.insert_time = simTime();
		unsigned int i;
		unsigned int c;
		for(i = 0; i < ports.size(); i++) { // for all input port number
			for(c = 0; c < entry.portList.size(); c++) { // search through whole port vector
				if (entry.portList.at(c) == ports.at(i)) { // if port number match
					break;
				}

			}
			if (c == entry.portList.size()) { // if port record not found
				entry.portList.push_back(ports.at(i)); // insert new
			}

		}
	}


	return;
}


/* --- PROTECTED --- */

void MACTable::initialize() {
	this->initDefaults();

	/* TEST */
/*
	tPortList tmp0, tmp1;
	tmp0.push_back(0);
	tmp0.push_back(1);
	tmp0.push_back(2);
	tmp0.push_back(3);

	tmp1.push_back(2);
	tmp1.push_back(3);
	tmp1.push_back(4);
	tmp1.push_back(5);

	addStatic(MACAddress("01:00:00:00:00:01"), tmp0);
	addStatic(MACAddress("01:00:00:00:00:02"), tmp1);
	removePort(MACAddress("01:00:00:00:00:01"), 2);
	remove(MACAddress("01:00:00:00:00:02"));
	add(MACAddress("01:00:00:00:00:03"), 16, GROUP, NONE);
*/
	/* END TEST */


	/* IEEE802.1D Table 7-10 Reserved addresses */
	  // Bridge Group Address -> go to STP
	add(MACAddress("01-80-C2-00-00-00"), 0, STATIC, STP);
	/* end of table */


	WATCH_MAP(this->table);
	WATCH(this->addressTableSize);
	WATCH(this->agingTime);
	WATCH_RW(uAgingTime);
	return;
}

void MACTable::finish() {
	EV << "ANSA Switch, end of transmission." << std::endl;
	return;
}



