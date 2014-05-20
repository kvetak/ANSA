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

#ifndef EIGRPNETWORKTABLE_H_
#define EIGRPNETWORKTABLE_H_

#include "IPv4Address.h"

/**
 * Network for EIGRP routing.
 */
class EigrpNetwork
{
  protected:
    int networkId;
    IPv4Address address;
    IPv4Address mask;

  public:
    EigrpNetwork(IPv4Address &address, IPv4Address &mask, int id) :
            networkId(id), address(address), mask(mask) {}

    const IPv4Address& getAddress() const {
        return address;
    }

    void setAddress(const IPv4Address& address) {
        this->address = address;
    }

    const IPv4Address& getMask() const {
        return mask;
    }

    void setMask(const IPv4Address& mask) {
        this->mask = mask;
    }

    int getNetworkId() const {
        return networkId;
    }

    void setNetworkId(int networkId) {
        this->networkId = networkId;
    }
};

/**
 * Table with networks for routing.
 */
class EigrpNetworkTable : cObject
{
  protected:
    std::vector<EigrpNetwork *> networkVec;
    int networkCnt;

  public:
    static const int UNSPEC_NETID = 0;

    EigrpNetworkTable() : networkCnt(1) { }
    virtual ~EigrpNetworkTable();

    EigrpNetwork *addNetwork(IPv4Address& address, IPv4Address& mask);
    EigrpNetwork *findNetworkById(int netId);
    std::vector<EigrpNetwork *> *getAllNetworks() { return &networkVec; }
    bool isAddressIncluded(IPv4Address& address, IPv4Address& mask);
    /**
     * Returns true if interface with specified address is contained in EIGRP.
     * @param resultNetId ID of network that belongs to the interface. If the interface does not
     * belong to any network, it has undefined value.
     */
    bool isInterfaceIncluded(const IPv4Address& ifAddress, const IPv4Address& ifMask, int *resultNetId);
};

#endif /* EIGRPNETWORKTABLE_H_ */
