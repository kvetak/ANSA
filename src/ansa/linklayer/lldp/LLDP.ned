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

package ansa.linklayer.lldp;

import inet.networklayer.contract.INetworkLayerLower;
//
// TODO auto-generated module
//
simple LLDP like INetworkLayerLower
{
    parameters:
    	@display("i=block/network2");

		int txCreditMax = default(5);
        int txFastInit = default(4);
        string adminStatus @enum("enabledRxTx", "enabledTxOnly", "enabledRxOnly", "disabled") = default("enabledRxTx");
        int msgFastTx = default(1);
        int msgTxHold = default(4);
        int msgTxInterval = default(30);
        int  reinitDelay = default(2);
        
        string routingTableModule = default("");		//?
        
        string deviceId = default("");
        string deviceType = default("Router");
        xml configFile = default(xmldoc("config.xml"));     
	gates:
        input ifIn[] @labels(INetworkDatagram/up);
        output ifOut[] @labels(INetworkDatagram/down);
}
