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

#ifndef ANSAINTERFACEENTRY_H_
#define ANSAINTERFACEENTRY_H_

#include "ansa/networklayer/common/VirtualForwarder.h"

namespace inet{
class INET_API ANSA_InterfaceEntry : public InterfaceEntry
{
    public:
        typedef std::vector<VirtualForwarder *> VirtualForwarderVector;

    protected:
        VirtualForwarderVector vforwarder;

        uint64_t bandwidth;   ///< Configured bandwidth in Kbps
        uint64_t delay;       ///< Configured delay in microseconds
        int reliability;    ///< Reliability (<1-255>), in percent (x/255)
        int recvLoad;       ///< Load for outgoing traffic (<1-255>), in percent (x/255)
        int transLoad;      ///< Load for incoming traffic (<1-255>), in percent (x/255)

    public:
        ANSA_InterfaceEntry(cModule *interfaceModule);
        virtual ~ANSA_InterfaceEntry() {};

        virtual std::string info() const override;

        virtual bool hasMacAddress(const MACAddress& addr) const;
        virtual bool hasIPAddress(const IPv4Address& addr) const;
        virtual const MACAddress& getMacVirtualForwarderById(int vforwarderId) const;
        virtual const MACAddress& getMacAddressByIP(const IPv4Address& addr) const;
        virtual int getVirtualForwarderId(const IPv4Address& addr);
        virtual int getVirtualForwarderId(const MACAddress& addr);
        virtual VirtualForwarder *getVirtualForwarderById(int id) { return vforwarder[id]; };

        /**
         * Adding Virtual Forwarder to this Interface
         * @param addr
         */
        virtual void addVirtualForwarder(VirtualForwarder* vforw);

        /**
         * Adding Virtual Forwarder from this Interface
         * @param addr
         */
        virtual void removeVirtualForwarder(VirtualForwarder* vforw);

        uint64_t getBandwidth() const     { return bandwidth; }
        uint64_t getDelay() const         { return delay; }
        int getReliability() const      { return reliability; }
        int getRecvLoad() const         { return recvLoad; }
        int getTransLoad() const        { return transLoad; }

        virtual void setDatarate(double d) override;
        void setBandwidth(uint64_t bandwidth){ if (this->bandwidth != bandwidth) { this->bandwidth = bandwidth; configChanged(NF_INTERFACE_CONFIG_CHANGED); } }
        void setDelay(uint64_t delay)        { if (this->delay != delay) { this->delay = delay; configChanged(NF_INTERFACE_CONFIG_CHANGED); } }
        void setReliability(int reliability) { if (this->reliability != reliability) { this->reliability = reliability; configChanged(NF_INTERFACE_CONFIG_CHANGED); } }
        void setRecvLoad(int recvLoad)       { if (this->recvLoad != recvLoad) { this->recvLoad = recvLoad; configChanged(NF_INTERFACE_CONFIG_CHANGED); } }
        void setTransLoad(int transLoad)     { if (this->transLoad != transLoad) { this->transLoad = transLoad; configChanged(NF_INTERFACE_CONFIG_CHANGED); } }
};
}
#endif /* ANSAINTERFACEENTRY_H_ */
