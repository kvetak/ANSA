//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_ANSAIP_H
#define __INET_ANSAIP_H

#include "QueueBase.h"
#include "InterfaceTableAccess.h"
#include "RoutingTableAccess.h"
#include "IRoutingTable.h"
#include "ICMPAccess.h"
#include "IPControlInfo.h"
#include "IPDatagram.h"
#include "IPFragBuf.h"
#include "ProtocolMap.h"
#include "ControlManetRouting_m.h"
#include "IP.h"


class ARPPacket;
class ICMPMessage;

enum AnsaIPProtocolId
{
    IP_PROT_PIM = 103
};



/**
 * Implements the IP protocol.
 */
class INET_API AnsaIP : public IP
{
  protected:

    /**
     * Handle IPDatagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handlePacketFromNetwork(IPDatagram *datagram);

  public:
    AnsaIP() {}

  protected:
    /**
     * Initialization
     */
    virtual void initialize();
};

#endif

