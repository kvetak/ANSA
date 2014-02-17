//
// Copyright (C) 2013 Brno University of Technology
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

#ifndef __INET_LISPCORE_H_
#define __INET_LISPCORE_H_

#include <omnetpp.h>
#include "UDPSocket.h"
#include "IPv4Address.h"
#include "AnsaRoutingTable.h"
#include "AnsaRoutingTableAccess.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"
#include "deviceConfigurator.h"

class LISPCore : public cSimpleModule, protected INotifiable
{
  protected:
    const char  *deviceId;   ///< Id of the device which contains this routing process.
    std::string hostName;    ///< Device name from the network topology.

    IInterfaceTable     *ift;                ///< Provides access to the interface table.
    AnsaRoutingTable    *rt;                 ///< Provides access to the IPv4 routing table.
    NotificationBoard   *nb;                 ///< Provides access to the notification board
    DeviceConfigurator  *devConf;

    UDPSocket socket;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void updateDisplayString();
};

#endif
