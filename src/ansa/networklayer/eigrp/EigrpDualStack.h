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
* @file EigrpDualStack.h
* @author Vit Rek, xrekvi00@stud.fit.vutbr.cz
* @date 27. 10. 2014
* @brief EIGRP Dual-stack (IPv4/IPv6) support header file
* @detail Functions prototypes for dual-stack (IPv4/IPv6) support
*/


#ifndef EIGRPDUALSTACK_H_
#define EIGRPDUALSTACK_H_

#include "networklayer/contract/ipv4/IPv4Address.h"
#include "networklayer/contract/ipv6/IPv6Address.h"
//#include "ANSAIPv6Address.h"

//#define DISABLE_EIGRP_IPV6


/**
 * Computes length of IPv4 netmask represented as address
 *
 * @param   netmask IPv4 netmask
 * @return  Length of netmask
 */
int getNetmaskLength(const inet::IPv4Address &netmask);

/**
 * Computes length of IPv6 netmask represented as address
 *
 * @param   netmask IPv6 netmask
 * @return  Length of netmask
 */
int getNetmaskLength(const inet::IPv6Address &netmask);

/**
 * Compare two IPv4 addresses masked by netmask
 *
 * @param   addr1   first address to compare
 * @param   addr2   second address to compare
 * @param   netmask network mask used for masking
 * @return  True if masked addresses are equal, otherwise false
 */
bool maskedAddrAreEqual(const inet::IPv4Address& addr1, const inet::IPv4Address& addr2, const inet::IPv4Address& netmask);

/**
 * Compare two IPv6 addresses masked by netmask
 *
 * @param   addr1   first address to compare
 * @param   addr2   second address to compare
 * @param   netmask network mask used for masking
 * @return  True if masked addresses are equal, otherwise false
 */
bool maskedAddrAreEqual(const inet::IPv6Address& addr1, const inet::IPv6Address& addr2, const inet::IPv6Address& netmask);

/**
 * Get prefix from IPv6 address and network mask (represented as address)
 *
 * @param   addr    address
 * @param   netmask network mask
 * @return  IPv6 network prefix
 */
inet::IPv6Address getPrefix(const inet::IPv6Address& addr, const inet::IPv6Address& netmask);

/**
 * Make network mask represented as IPv6 address from netmask length
 *
 * @param   mask length
 * @return  network mask represented as IPv6 address
 *
 * @note    Instead of this function, you can simply use: inet::IPv6Address(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff).getPrefix(prefixLength)
 */
inet::IPv6Address makeNetmask(int length);

#endif /* EIGRPDUALSTACK_H_ */
