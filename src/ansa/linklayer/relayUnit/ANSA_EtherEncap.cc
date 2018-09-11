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

#include "ansa/linklayer/relayUnit/ANSA_EtherEncap.h"
//#include "inet/common/INETUtils.h"
//#include "inet/common/ModuleAccess.h"
//#include "inet/common/ProtocolTag_m.h"
//#include "inet/common/checksum/EthernetCRC.h"
//#include "inet/linklayer/common/FcsMode_m.h"
//#include "inet/linklayer/common/Ieee802Ctrl.h"
//#include "inet/linklayer/common/Ieee802SapTag_m.h"
//#include "inet/linklayer/common/InterfaceTag_m.h"
//#include "inet/linklayer/common/MacAddressTag_m.h"
//#include "inet/linklayer/ethernet/EtherEncap.h"
//#include "inet/linklayer/ethernet/EtherFrame_m.h"
//#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
//#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
//#include "inet/networklayer/contract/IInterfaceTable.h"

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

Define_Module(ANSA_EtherEncap);

void ANSA_EtherEncap::initialize(int stage)
{

    EtherEncap::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER)
    {
        //register sservice and protocol
        registerService(Protocol::ethernetMac, gate("upperLayerIn"), nullptr);
//        registerService
        registerProtocol(Protocol::ethernetMac, nullptr, gate("upperLayerOut"));

        //TODO register for 'commandFromHigherLayer' as well?

    }


}

//void ANSA_EtherEncap::handleMessage(cMessage *msg)
//{
//    // TODO - Generated method body
//}

} //namespace
