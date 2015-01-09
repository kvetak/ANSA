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
* @file VRRPv2VirtualRouter.h
* @author Petr Vitek
* @brief
* @detail
*/
#ifndef VRRPV2VIRTUALROUTER_H_
#define VRRPV2VIRTUALROUTER_H_

#include <omnetpp.h>

#include "AnsaARP.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"
#include "AnsaInterfaceEntry.h"
#include "IPv4Address.h"
#include "MACAddress.h"
#include "VRRPv2Advertisement_m.h"


class VRRPv2VirtualRouter : public cSimpleModule
{
    protected:
        /**
         * statistic signal
         */
        static simsignal_t vrStateSignal;
        static simsignal_t sentARPSignal;
        static simsignal_t sentAdverSignal;
        static simsignal_t recvAdverSignal;

    public:
        VRRPv2VirtualRouter();
        virtual ~VRRPv2VirtualRouter();

        enum VRRPState {
            SHUTDOWN = 0,
            INITIALIZE,
            BACKUP,
            MASTER,
        };

        enum VRRPArpType {
            REQUEST = 1,
            REPLY   = 2,
        };

        enum VRRPPhase {
            INIT,
            TIMER_START,
            TIMER_END,
            LEAVE,
            STOP,
        };

        enum VRRPTimer {
            ADVERTISE,
            MASTERDOWN,
            BROADCAST,
            PREEMTION,
            INITCHECK
        };

    protected:
        VRRPState   state;

        const char*         hostname;
        IInterfaceTable     *ift;
        AnsaInterfaceEntry  *ie;
        AnsaARP             *arp;
        VirtualForwarder    *vf;

        IPv4Address multicastIP;
        MACAddress  virtualMAC;
        VRRPArpType arpType;
        bool        own;
        std::string description;
        double      arpDelay;

        int         vrid;
        IPv4Address IPprimary;
        std::vector<IPv4Address> IPsecondary;
        int         version;
        int         priority;
        int         protocol;
        int         ttl;
        bool        preemtion;
        bool        preemtionDelay;
        bool        learn;

        int         advertisementIntervalActual;
        int         advertisementInterval;

        simtime_t   masterDownTimerInit;
        simtime_t   adverTimerInit;
        simtime_t   preemTimerInit;
        cMessage*   masterDownTimer;
        cMessage*   adverTimer;
        cMessage*   broadcastTimer;
        cMessage*   preemtionTimer;


        cMessage*   initCheckTimer;
        void scheduleInitCheck();


    public:

        /** @name Field setters */
        //@{
        virtual void setArp(int a) { arpType = (enum VRRPArpType) a; };
        virtual void setIPPrimary(IPv4Address addr) { IPprimary = addr; };
        virtual void setIPPrimary(const char* addr) { IPprimary.set(addr); };
        virtual void setPriority(int p) { priority = p; };
        virtual void setPreemtion(bool p) { preemtion = p; };
        virtual void setLearn(bool l) { learn = l; }
        virtual void setAdvertisement(int advert) { advertisementInterval = advert; };
        virtual void addIPSecondary(IPv4Address addr) { IPsecondary.push_back(addr); };
        virtual void addIPSecondary(const char* addr) { IPsecondary.push_back(IPv4Address(addr)); };
        virtual void setDescription(std::string d) { description = d; };
        virtual void setPreemtionDelay(int d) { preemtionDelay = true; preemTimerInit = d; }
        //@}

        /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
        //@{
        virtual AnsaInterfaceEntry* getInterface() { return ie; };
        IPv4Address getPrimaryIP() { return IPprimary; };
        MACAddress getVirtualMAC() { return virtualMAC; }
        int getVrid() const { return vrid; };
        int getArp() { return arpType; };
        int getPriority() { return priority; };
        bool getPreemtion() { return preemtion; };
        bool getLearn() { return learn; };
        std::string getStrOfBool(bool value) const { return ((value) ? "Y" : "-"); };
        std::string getStrOfState(VRRPState state) const;
        simtime_t getAdvertisementInterval() { return advertisementIntervalActual; };
        //@}

        /** @name Field debug messages  */
        //@{
        virtual std::string info() const;
        virtual std::string detailedInfo() const;
        virtual std::string showBrief() const;
        virtual std::string showConfig() const;
        //@}

        /**
         * The virtual mac address
         */
        virtual void createVirtualMAC();

        /**
         * Calculating checksum message
         * @param version
         * @param vrid
         * @param priority
         * @param advert
         * @param address
         * @return
         */
        virtual uint16_t getAdvertisementChecksum(int version, int vrid, int priority, int advert, std::vector<IPv4Address> address);


    protected:

        /**
         * Calculation Skew interval
         * @return
         */
        simtime_t getSkewTime() { return (((256.0 - priority) * advertisementIntervalActual) / 256.0); };

        /**
         * Calculation Master Down interval
         * @return
         */
        simtime_t getMasterDownInterval() { return (3 * advertisementIntervalActual + this->getSkewTime()); };

        /**
         * Activity in the state Initialize
         */
        void stateInitialize();

        /**
         * Activity in the state Backup
         * @param phase
         */
        void stateBackup(VRRPPhase phase);

        /**
         * Activity in the state Master
         * @param phase
         */
        void stateMaster(VRRPPhase phase);

        /** @name Management timers  */
        //@{
        virtual void handleTimer(cMessage *msg);
        virtual void cancelTimer(cMessage* timer);
        //@}

        /** @name Advertisement Timer  */
        //@{
        virtual void scheduleAdverTimer();
        virtual void cancelAdverTimer();
        //@}

        /** @name Master Down Timer  */
        //@{
        virtual void scheduleMasterDownTimer();
        virtual void cancelMasterDownTimer();
        //@}

        /** @name Preemtion Delay Timer  */
        //@{
        virtual void schedulePreemDelayTimer();
        virtual void cancelPreemDelayTimer();
        //@}

        /** @name Broadcast Timer  */
        //@{
        virtual void scheduleBroadcastTimer();
        virtual void cancelBroadcastTimer();
        //@}

        /**
         * Send ADVERTISEMENT packet to multicast group.
         */
        virtual void sendAdvertisement();

        /**
         * Sends ARP packets to all IP addresses associated with the virtual router.
         */
        virtual void sendBroadcast();

        /**
         * Processing of received messages.
         * @param msg
         */
        virtual void handleMessage(cMessage *msg);

        /**
         * Check the values ​​received ADVERTISEMENT. If everything is okay returns true.
         * @param msg
         * @return
         */
        virtual bool handleAdvertisement(VRRPv2Advertisement* msg);

        /**
         * Packet processing ADVERTISEMENT in state backup.
         * @param msg
         */
        virtual void handleAdvertisementBackup(VRRPv2Advertisement* msg);

        /**
         * Packet processing ADVERTISEMENT in state master.
         * @param msg
         */
        virtual void handleAdvertisementMaster(VRRPv2Advertisement* msg);

        virtual int numInitStages() const { return 5; };
        virtual void initialize(int stage);

        /**
         * The default configuration.
         */
        virtual void loadDefaultConfig();

        /**
         * If the IP address of the interface identical to the virtual IP address defines the property.
         */
        virtual void setOwn();

        /** @name Field debug messages  */
        //@{
        virtual std::string debugEvent() const;
        virtual std::string debugStateMachine(std::string from, std::string to) const;
        virtual std::string debugPacketMaster(uint16_t checksum) const;
        virtual std::string debugPacketBackup(int priority, IPv4Address ipaddr) const;
        //@}
};

#endif /* VRRPV2VIRTUALROUTER_H_ */
