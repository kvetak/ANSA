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

#include "LISPCore.h"

Define_Module(LISPCore);

void LISPCore::initialize()
{
    // access to the routing and interface table
    rt = AnsaRoutingTableAccess().get();
    ift = InterfaceTableAccess().get();
    // subscribe for changes in the device
    nb = NotificationBoardAccess().get();
    // get deviceId
    deviceId = par("deviceId");
    // read the RIP process configuration
    devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    //devConf->loadLISPConfig(this);
    //socket.setOutputGate(gate("udpOut"));
    //socket.bind(4341);
    //setSocketOptions();
}

void LISPCore::handleMessage(cMessage *msg)
{

}

void LISPCore::updateDisplayString()
{
    if (ev.isGUI())
    {
        //TODO
    }
}

void LISPCore::receiveChangeNotification(int category, const cObject *details)
{
    //TODO
}

