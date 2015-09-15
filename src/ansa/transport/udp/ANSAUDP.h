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

/**
 * @file ANSAUDP.h
 * @date 21.5.2013
 * @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Extended UDP
 * @details Bind method in this class allows bind only to the specified port and than bind to
 *          that port again but with the specified address
 */

#ifndef __INET_ANSA_ANSAUDP_H
#define __INET_ANSA_ANSAUDP_H

#include "transportlayer/udp/UDP.h"

/**
 * Created for the RIPng class - the bind() method had to be reimplemented using method
 * ANSAfindSocketByLocalAddress() instead of findSocketByLocalAddress()
 * TODO: watch this class and changes in INET! - especially UDP class
 * @see UDP
 */
class ANSAUDP : public inet::UDP
{
    public:
        ANSAUDP();
        virtual ~ANSAUDP();

    protected:
        virtual void bind(int sockId, int gateIndex, const inet::L3Address& localAddr, int localPort);
        virtual SockDesc *ANSAfindSocketByLocalAddress(const inet::L3Address& localAddr, ushort localPort);
};

#endif /* __INET_ANSA_ANSAUDP_H */
