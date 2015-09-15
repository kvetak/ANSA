// Copyright (C) 2011 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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

/**
 * @file ipSplitter.cc
 * @author Marek Cerny, Matej Hrncirik, Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 2011
 * @brief Splits transport protocols to appropriates modules
 * @detail Splits transport protocols to appropriates modules
 */

#include "ansa/networklayer/ipSplitter/ipSplitter.h"

Define_Module(IpSplitter);

void IpSplitter::initialize(){

}

void IpSplitter::handleMessage(cMessage *msg){

    cGate* gate = msg->getArrivalGate();
    std::string gateName = gate->getBaseName();
    int gateIndex = gate->getIndex();

    // packet coming from network layer modules within the router
    if(gateName == "isisIn"){

        inet::Ieee802Ctrl *ctrl = new inet::Ieee802Ctrl();
        ctrl->setDsap(inet::SAP_CLNS);
        ctrl->setSsap(inet::SAP_CLNS);

        inet::MACAddress ma;

        ISISMessage *isisMsg = (ISISMessage *) msg;
        switch(isisMsg->getType()){
            case LAN_L1_HELLO:
            case L1_CSNP:
            case L1_PSNP:
            case L1_LSP:
                ma.setAddress(ISIS_ALL_L1_IS);
                break;

            case LAN_L2_HELLO:
            case L2_CSNP:
            case L2_PSNP:
            case L2_LSP:
                ma.setAddress(ISIS_ALL_L2_IS);
                break;

            case PTP_HELLO:
                if(((ISISPTPHelloPacket *)isisMsg)->getCircuitType() == L1_TYPE){
                    ma.setAddress(ISIS_ALL_L1_IS);
                }else{
                    ma.setAddress(ISIS_ALL_L2_IS);
                }
                break;
        }

        ctrl->setDest(ma);
        isisMsg->setControlInfo(ctrl);
        this->send(msg, "ifOut", gateIndex);

    }else if (gateName == "ipv4In" || gateName == "ipv6In")
    {


        // send packet to out interface
        this->send(msg, "ifOut", gateIndex);

    // packet coming from in interfaces
    }
    else
    {
        // IPv6 datagram, send it to networkLayer6 via ipv6Out
        if (dynamic_cast<inet::IPv6Datagram *>(msg))
        {
            //NetworkLayer6 is connected
            if (this->gate("ipv6Out", gateIndex)->isConnected())
                this->send(msg, "ipv6Out", gateIndex);
            //NetworkLayer6 is probably missing, hence not a dual-stack
            else {
                delete msg;
                return;
            }
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
