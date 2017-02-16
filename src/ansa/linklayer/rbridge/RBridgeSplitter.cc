// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file RBridgeSplitter.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.2.2013
 * @brief Handles de/encapsulation for IS-IS module and splits traffic between IS-IS and TRILL modules.
 * @detail Handles de/encapsulation for IS-IS module and splits traffic between IS-IS and TRILL modules.
 * @todo Z9
 */

#include "RBridgeSplitter.h"

namespace inet {


Define_Module(RBridgeSplitter);

void RBridgeSplitter::initialize(int stage){

    if(stage == 3){
      //TODO A!
//        trillModule = TRILLAccess().get();

//        isisModule = ISISAccess().get();

//        this->vlanTableModule = ModuleAccess<RBVLANTable>("rBVLANTable").get();

//        this->portTableModule = ModuleAccess<RBPortTable>("rBVPortTable").get();

//        this->ift = InterfaceTableAccess().get();
    }
}

void RBridgeSplitter::handleMessage(cMessage *msg)
{

    cGate* gate = msg->getArrivalGate();
    std::string gateName = gate->getBaseName();
    int gateIndex = gate->getIndex();

    // packet coming from network layer modules within the router
 if (gateName == "upperLayerIn" || gateName == "trillIn")
    {
        this->send(msg, "ifOut", gateIndex);
    }
    else
    {
        if (dynamic_cast<AnsaEtherFrame *>(msg))
        {
//            EthernetIIFrame * frame = (EthernetIIFrame *) msg;
            AnsaEtherFrame * frame = (AnsaEtherFrame *) msg;
            //if src unicast
            //trillModule->learn(msg);

            //check integrity, ...
            this->send(msg, "trillOut", gateIndex);

        }

        else if (dynamic_cast<EthernetIIFrame *>(msg))
        {
            EthernetIIFrame * frame = (EthernetIIFrame *) msg;
            //if src unicast
            //trillModule->learn(msg);

            //check integrity, ...

            //if ethertype == L2_ISIS AND Outer.MacDA == ALL-IS-IS-RBridges
            if (frame->getEtherType() == ETHERTYPE_L2_ISIS)
            {
                trillModule->learn(frame);
                this->send(msg, "isisOut", gateIndex);

                return;
            }

            else
            {
                this->send(msg, "trillOut", gateIndex);
            }
        }
        else
        {
            EV<< "Warning: received unsupported frame type" << endl;
        }
    }

}





}
