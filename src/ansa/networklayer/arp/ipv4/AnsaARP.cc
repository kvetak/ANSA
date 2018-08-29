/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
 * Copyright (C) 2014 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "ansa/networklayer/arp/ipv4/AnsaARP.h"

#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "ansa/routing/glbp/GLBPVirtualForwarder.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"

namespace inet {

Define_Module(AnsaARP);

simsignal_t AnsaARP::recvReqSignal = registerSignal("recvRequest");

MACAddress AnsaARP::getMacAddressForArpReply(InterfaceEntry *ie, ARPPacket *arp)
{
    if ( dynamic_cast<ANSA_InterfaceEntry *>(ie) != nullptr ) {
        ANSA_InterfaceEntry *aie = dynamic_cast<ANSA_InterfaceEntry *>(ie);
        int vfn = aie->getVirtualForwarderId(arp->getDestIPAddress());
        VirtualForwarder *vf = (vfn == -1 ? nullptr : aie->getVirtualForwarderById(vfn) );

        //is it GLBP protocol?
        if( (vfn != -1) && (dynamic_cast<GLBPVirtualForwarder *>(vf) != nullptr) ){
            GLBPVirtualForwarder *GLBPVf = dynamic_cast<GLBPVirtualForwarder *>(vf);
            MACAddress myMACAddress;

            //arp req to AVG?
            if (GLBPVf->isAVG()){
                emit(recvReqSignal,true);
                myMACAddress = GLBPVf->getMacAddress();
                if (myMACAddress.compareTo(MACAddress("00-00-00-00-00-00")) == 0){
                    return myMACAddress;
                }
            }else if (!GLBPVf->isDisable()){
                return MACAddress::UNSPECIFIED_ADDRESS;
            }
            return myMACAddress;
        }else{
            return ie->getMacAddress();
        }
    }
    else {
        return ie->getMacAddress();
    }
}

} // namespace inet

