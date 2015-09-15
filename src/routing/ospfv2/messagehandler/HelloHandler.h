//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_HELLOHANDLER_H
#define __INET_HELLOHANDLER_H

#include "routing/ospfv2/messagehandler/IMessageHandler.h"

namespace inet {

namespace ospf {

class HelloHandler : public IMessageHandler
{
  public:
    HelloHandler(Router *containingRouter);

    void processPacket(OSPFPacket *packet, Interface *intf, Neighbor *unused = nullptr) override;
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_HELLOHANDLER_H
