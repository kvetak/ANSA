/*
 * Copyright (C) 2018 OpenSim Ltd.
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

#ifndef __ANSA_ANSAARP_H
#define __ANSA_ANSAARP_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/arp/ipv4/ARP.h"

namespace inet {

/**
 * ARP implementation.
 */
class INET_API AnsaARP : public ARP
{
  public:
    static simsignal_t recvReqSignal;
  protected:
    virtual MACAddress getMacAddressForArpReply(InterfaceEntry *ie, ARPPacket *arp) override;
};

} // namespace inet

#endif // ifndef __ANSA_ANSAARP_H
