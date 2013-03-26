//
// Marek Cerny, 2MSK
// FIT VUT 2011
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

#include "ipSplitter.h"

Define_Module(IpSplitter);

void IpSplitter::initialize(){

}

void IpSplitter::handleMessage(cMessage *msg){

    cGate* gate = msg->getArrivalGate();
    std::string gateName = gate->getBaseName();
    int gateIndex = gate->getIndex();

    // packet coming from network layer modules within the router
    if (gateName == "ipv4In" || gateName == "ipv6In" || gateName == "isisIn")
    {

        // send packet to out interface
        this->send(msg, "ifOut", gateIndex);

    // packet coming from in interfaces
    }
    else
    {

        // IPv6 datagram, send it to networkLayer6 via ipv6Out
        if (dynamic_cast<IPv6Datagram *>(msg))
        {
            this->send(msg, "ipv6Out", gateIndex);
        }
        else
        {
            if(dynamic_cast<ISISMessage *>(msg))
            {
                this->send(msg, "isisOut", gateIndex);
            }

            // other (IPv4), send it to networkLayer via ipv4Out
            else
            {
                this->send(msg, "ipv4Out", gateIndex);
            }
        }
    }
}
