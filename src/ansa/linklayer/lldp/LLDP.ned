//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
//
// @file LLDP.ned
// @author Tomas Rajca
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package ansa.linklayer.lldp;

import ansa.linklayer.lldp.tables.LLDPAgentTable;
import ansa.linklayer.lldp.tables.LLDPNeighbourTable;
import inet.networklayer.contract.INetworkLayerLower;


module LLDP like INetworkLayerLower
{
    parameters:
        @display("i=block/network2");

        string deviceId = default("");
        string deviceType = default("Router");
        xml configFile = default(xmldoc("config.xml"));

        **.deviceId = deviceId;
        **.deviceType = deviceType;
        **.configFile = default(xmldoc("config.xml"));

        int txCreditMax = default(5);
        int txFastInit = default(3);
        string adminStatus @enum("enabledRxTx","enabledTxOnly","enabledRxOnly","disabled") = default("enabledRxTx");
        int msgFastTx = default(1);
        int msgTxHold = default(4);
        int msgTxInterval = default(30);
        int reinitDelay = default(2);

        *.txCreditMax = txCreditMax;
        *.txFastInit = txFastInit;
        *.adminStatus = adminStatus;
        *.msgFastTx = msgFastTx;
        *.msgTxHold = msgTxHold;
        *.msgTxInterval = msgTxInterval;
        *.reinitDelay = reinitDelay;
        *.lldpNeighbourTableModule = default("^.lldpNeighbourTable");
        *.lldpAgentTableModule = default("^.lldpAgentTable");
    gates:
        input ifIn @labels(INetworkDatagram/up);
        output ifOut @labels(INetworkDatagram/down);


    submodules:
        lldpAgentTable: LLDPAgentTable {
            parameters:
                @display("p=202,76");
        }

        lldpNeighbourTable: LLDPNeighbourTable {
            parameters:
                @display("p=321,76");
        }

        lldpMain: LLDPMain {
            parameters:
                @display("p=88,76");
        }

    connections:
        ifIn --> { @display("m=w"); } --> lldpMain.ifIn;
        lldpMain.ifOut --> { @display("m=w"); } --> ifOut;
}
