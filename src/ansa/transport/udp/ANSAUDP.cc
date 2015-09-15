// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

/**
 * @file ANSAUDP.cc
 * @date 21.5.2013
 * @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Extended UDP
 * @details Bind method in this class allows bind only to the specified port and than bind to
 *          that port again but with the specified address
 */

#include "ansa/transport/udp/ANSAUDP.h"

Define_Module(ANSAUDP);

ANSAUDP::ANSAUDP()
{
}

ANSAUDP::~ANSAUDP()
{
}

void ANSAUDP::bind(int sockId, int gateIndex, const inet::L3Address& localAddr, int localPort)
{
    if (sockId == -1)
        error("sockId in BIND message not filled in");

    if (localPort<-1 || localPort>65535) // -1: ephemeral port
        error("bind: invalid local port number %d", localPort);

    // do not allow two apps to bind to the same address/port combination
    SockDesc *existing = ANSAfindSocketByLocalAddress(localAddr, localPort);
    if (existing != NULL)
        error("bind: local address/port %s:%u already taken", localAddr.str().c_str(), localPort);

    SocketsByIdMap::iterator it = socketsByIdMap.find(sockId);
    if (it != socketsByIdMap.end())
    {
        SockDesc *sd = it->second;
        if (sd->isBound)
            error("bind: socket is already bound (sockId=%d)", sockId);

        sd->isBound = true;
        sd->localAddr = localAddr;
        if (localPort != -1 && sd->localPort != localPort)
        {
            socketsByPortMap[sd->localPort].remove(sd);
            sd->localPort = localPort;
            socketsByPortMap[sd->localPort].push_back(sd);
        }
    }
    else
    {
        SockDesc *sd = createSocket(sockId, gateIndex, localAddr, localPort);
        sd->isBound = true;
    }
}

inet::UDP::SockDesc *ANSAUDP::ANSAfindSocketByLocalAddress(const inet::L3Address& localAddr, ushort localPort)
{
    SocketsByPortMap::iterator it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return NULL;

    SockDescList& list = it->second;
    for (SockDescList::iterator it = list.begin(); it != list.end(); ++it)
    {
        SockDesc *sd = *it;
        /*this condition had to be edited, because RIPng need one socket
         *per interface for sending messages and one socket (bind to RIPng
         *port only) for receiving messages
         *otherwise - bind method would not allowed that
         */
        if (sd->localAddr == localAddr)
            return sd;
    }
    return NULL;
}

