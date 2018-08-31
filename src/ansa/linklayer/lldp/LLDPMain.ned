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
// @file LLDPMain.ned
// @author Tomas Rajca
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package ansa.linklayer.lldp;

simple LLDPMain
{
    parameters:
        @display("i=block/dispatch");
		int txCreditMax;
        int txFastInit;
        string adminStatus @enum("enabledRxTx", "enabledTxOnly", "enabledRxOnly", "disabled");
        int msgFastTx;
        int msgTxHold;
        int msgTxInterval;
        int  reinitDelay;
        
        string deviceId = default("");
        string deviceType = default("");
        xml configFile = default(xmldoc("config.xml"));

        string lldpAgentTableModule;
        string lldpNeighbourTableModule;
    gates:
        input ifIn;
        output ifOut;
}

