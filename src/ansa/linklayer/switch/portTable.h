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

#ifndef __PORTTABLE_H__
#define __PORTTABLE_H__

#include <omnetpp.h>
#include <vector>

class PortTable : public cSimpleModule
{
  public:

	  typedef enum e_port_state {
		  OFF = 0,
		  BLOCKING = 1,
		  DISCARTING = 2,
		  LEARNING = 3,
		  FORWARDING = 4,
	  } tPortState;

	  typedef struct s_port {
		  tPortState state;
	  } tPort;

	  typedef std::vector<tPort> tPortList;

	  /* PUBLIC ACCESS */

	  tPort getPort(int);
	  tPortState getState(int);

	  /* STP PROCESS MANIPULATION */
	  void setState(int, tPortState);
	  void setState(std::vector<int>&, tPortState);
	  void allState(tPortState);
	  void allOff();
	  void off(std::vector<int>&);
	  void forwarding(std::vector<int>&);


  private:
	  int portCount;
	  tPortList list;

	  void initDefault();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);


};

inline std::ostream& operator<<(std::ostream& os, const PortTable::tPortState s) {

	switch (s){
	case PortTable::OFF:
		os << "OFF";
		break;
	case PortTable::BLOCKING:
		os << "BLOCKING";
		break;
	case PortTable::DISCARTING:
		os << "DISCARTING";
		break;
	case PortTable::LEARNING:
		os << "LEARNING";
		break;
	case PortTable::FORWARDING:
		os << "FORWARDING";
		break;
	default:
		os << "<???>";
		break;
	}

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const PortTable::tPort p) {
	os << "PortState: " << p.state;
	return os;
}

#endif
