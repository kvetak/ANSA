/*
 * pimSplitter.h
 *
 *  Created on: 3.12.2011
 *      Author: Haczek
 */

#ifndef PIMSPLITTER_H_
#define PIMSPLITTER_H_

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"
#include "IPControlInfo.h"
#include "AnsaInterfaceTableAccess.h"
#include "AnsaInterfaceTable.h"
#include "AnsaRoutingTable.h"
#include "AnsaIP.h"


class pimSplitter : public cSimpleModule
{
	private:
		IRoutingTable           	*rt;           /**< Odkaz na smerovaci tabulku. */
	    IInterfaceTable         	*ift;          /**< Odkaz na tabulku rozhrani. */
	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};


#endif /* PIMSPLITTER_H_ */
