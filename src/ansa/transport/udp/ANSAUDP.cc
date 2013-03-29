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

#include "ANSAUDP.h"

Define_Module(ANSAUDP);

ANSAUDP::ANSAUDP()
{
}

ANSAUDP::~ANSAUDP()
{
}

UDP::SockDesc *ANSAUDP::findSocketByLocalAddress(const IPvXAddress& localAddr, ushort localPort)
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

