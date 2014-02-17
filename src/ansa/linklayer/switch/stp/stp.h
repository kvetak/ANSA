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

#ifndef __ANSAINET_STP_H_
#define __ANSAINET_STP_H_

#include <string>
#include <vector>

#include <omnetpp.h>
//#include "MACAddress.h"
#include "stpi.h"
#include "MACAddress.h"
#include "macTable.h"
#include "STPBPDU_m.h"
#include "STPTCN_m.h"

class Stp : public cSimpleModule
{
  public:

	bool learning(unsigned int, unsigned int);
	bool forwarding(unsigned int, unsigned int);

	void dispatchALL();
	void dispatch(unsigned int);


	/* FOR XML CONFIGURATOR*/
	int getInstanceIndex(unsigned int);
	void setBridgePriority(unsigned int, unsigned int);

	void setPortPriority(unsigned int, std::vector<unsigned int>&, std::vector<unsigned int>&);
	void setLinkCost(unsigned int, std::vector<unsigned int>&, std::vector<unsigned int>&);
	void setForwardDelay(unsigned int, unsigned int);
	void setMaxAge(unsigned int, unsigned int);
	void setHelloTime(unsigned int, unsigned int);




  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage * msg);
    virtual void finish();
    void handleSelfMessage(cMessage * msg);
    void handleBPDU(STPBPDU * bpdu);
    void handleTCN(STPTCN * tcn);

    /* internal for XML config */
    void setPortPriority(unsigned int, unsigned int, unsigned int);
    void setLinkCost(unsigned int, unsigned int, unsigned int);

	unsigned int portCount;
	MACAddress bridgeAddress;


    std::vector<unsigned int> instReg; // search vector of existing instances
    std::vector<stpi> inst; // actual instances

	/* timers */
    cMessage * tick;

    MACTable * addrTable;

};






#endif
