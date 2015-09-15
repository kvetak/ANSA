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
* @file EigrpDualStack.cc
* @author Vit Rek, xrekvi00@stud.fit.vutbr.cz
* @date 27. 10. 2014
* @brief EIGRP Dual-stack (IPv4/IPv6) support
* @detail Contains functions for dual-stack (IPv4/IPv6) support
*/

#include "ansa/networklayer/eigrp/EigrpDualStack.h"

/**
 * Uses inet::IPv4Address.getNetmaskLength() method
 */
int getNetmaskLength(const inet::IPv4Address &netmask)
{
    return netmask.getNetmaskLength();
}


/**
 * Uses four times inet::IPv4Address.getNetmaskLength() method on four parts of IPv6 address
 */
int getNetmaskLength(const inet::IPv6Address &netmask)
{
    int length = 0;

    for(int i = 0; i <= 3; ++i)
    {
        //length += inet::IPv4Address(netmask.words()[i]).getNetmaskLength();

        length += (static_cast<inet::IPv4Address> (netmask.words()[i])).getNetmaskLength();      //TODO - verify!
    }

    return length;
}

bool maskedAddrAreEqual(const inet::IPv4Address& addr1, const inet::IPv4Address& addr2, const inet::IPv4Address& netmask)
{
    //return !(bool)((addr1.addr ^ addr2.addr) & netmask.addr);
    return inet::IPv4Address::maskedAddrAreEqual(addr1, addr2, netmask);
}

bool maskedAddrAreEqual(const inet::IPv6Address& addr1, const inet::IPv6Address& addr2, const inet::IPv6Address& netmask)
{
    const uint32 *a1 = addr1.words();
    const uint32 *a2 = addr2.words();
    const uint32 *mask = netmask.words();

    return !(static_cast<bool> (
            ((a1[0] ^ a2[0]) & mask[0]) |
            ((a1[1] ^ a2[1]) & mask[1]) |
            ((a1[2] ^ a2[2]) & mask[2]) |
            ((a1[3] ^ a2[3]) & mask[3]))
            );//TODO - verify!


}

inet::IPv6Address getPrefix(const inet::IPv6Address& addr, const inet::IPv6Address& netmask)
{
    const uint32 *addrp = addr.words();
    const uint32 *netmaskp = netmask.words();

    return inet::IPv6Address(addrp[0] & netmaskp[0], addrp[1] & netmaskp[1], addrp[2] & netmaskp[2], addrp[3] & netmaskp[3]); //TODO - verify
}

inet::IPv6Address makeNetmask(int length) //TODO - verify
{
    uint32 netmask[4] = {0, 0, 0, 0};

    for(int i = 0; i < 4; ++i)
    {//through 4 parts of address
        int wlen = length - (i * 32);   //computes number of ones bits in part

        if(wlen > 0)
        {//some bits to set
            netmask[i] = (wlen >= 32) ? 0xffffffffu : ~(0xffffffffu >> wlen);   //(Implementation note: MSVC refuses to shift by 32 bits!)
        }
        else
        {// nothing to set
            break;
        }
    }

    return inet::IPv6Address(netmask[0], netmask[1], netmask[2], netmask[3]);
}
