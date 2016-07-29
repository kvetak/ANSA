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
* @file VRRPv2VirtualRouter.cc
* @author Petr Vitek
* @brief
* @detail
*/
#include "ansa/routing/vrrp/VRRPv2VirtualRouter.h"
#include "ansa/routing/vrrp/VRRPv2DeviceConfigurator.h"

#include "inet/common/ModuleAccess.h"
//#include "InterfaceTableAccess.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

Define_Module(VRRPv2VirtualRouter);

simsignal_t VRRPv2VirtualRouter::vrStateSignal;
simsignal_t VRRPv2VirtualRouter::sentARPSignal;
simsignal_t VRRPv2VirtualRouter::sentAdverSignal;
simsignal_t VRRPv2VirtualRouter::recvAdverSignal;

VRRPv2VirtualRouter::VRRPv2VirtualRouter()
{
    adverTimer = NULL;
    masterDownTimer = NULL;
    broadcastTimer = NULL;
    preemtionTimer = NULL;
    initCheckTimer = NULL;
    vf = NULL;
    arp = NULL;
}

VRRPv2VirtualRouter::~VRRPv2VirtualRouter()
{
    cancelAndDelete(masterDownTimer);
    cancelAndDelete(broadcastTimer);
    cancelAndDelete(preemtionTimer);
    cancelAndDelete(adverTimer);

    cancelAndDelete(initCheckTimer);
}

void VRRPv2VirtualRouter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        vrStateSignal = registerSignal(VRSTATE_SIG);
        sentARPSignal = registerSignal(SENTARP_SIG);
        sentAdverSignal = registerSignal(SENTADV_SIG);
        recvAdverSignal = registerSignal(RECVADV_SIG);

        WATCH(state);
        WATCH_PTR(vf);
        WATCH(advertisementInterval);
        WATCH(advertisementIntervalActual);
        WATCH(priority);
        WATCH(preemtion);

        ift = getModuleFromPar<IInterfaceTable>(par(IFT_PAR), this);
        arp = getModuleFromPar<ARP>(par(ARP_PAR), this);
        ie = dynamic_cast<ANSA_InterfaceEntry*>(ift->getInterfaceById((int) par(INTERFACE_PAR)));

        //set default configuration
        loadDefaultConfig();

        //load the Virtual Router process configuration
        VRRPv2DeviceConfigurator* devConf = new VRRPv2DeviceConfigurator(par(CONFIG_PAR).xmlValue(),ift);
        devConf->loadVRRPv2VirtualRouterConfig(this);
        EV << hostname << "(vrrp-configuration)# " << showConfig() << endl;

        //join multicast goup
        ie->ipv4Data()->joinMulticastGroup(multicastIP);

        //virtual mac
        createVirtualMAC();

        vf = new VirtualForwarder();
        vf->addIPAddress(IPprimary);
        vf->setMACAddress(virtualMAC);
        ie->addVirtualForwarder(vf);

        setOwn();
        stateInitialize();

        IPSocket ipSocket(gate(IPOUT_GATE));
        ipSocket.registerProtocol(IP_PROT_VRRP);
    }
}

/***
 *  STATE MACHINE
 */
void VRRPv2VirtualRouter::stateInitialize()
{
    state = INITIALIZE;
    if (!ie->isUp()) {
        scheduleInitCheck();
        return;
    }

    emit(vrStateSignal, INITIALIZE);

    if (priority == ((int) par(PRIOOWN_PAR)))
    {
        sendAdvertisement();
        scheduleBroadcastTimer();
//      sendBroadcast();

        adverTimerInit = getAdvertisementInterval();
        EV << hostname << "# " << debugStateMachine("Init", "Master") << endl;

        stateMaster(INIT);
    }
    else
    {
        masterDownTimerInit = getMasterDownInterval();
        EV << hostname << "# " << debugStateMachine("Init", "Backup") << endl;
        stateBackup(INIT);
    }
}

void VRRPv2VirtualRouter::stateBackup(VRRPPhase phase)
{
    //Check whether interface is disconnected
    //IF yes THEN transit to INIT state
    if (!ie->isUp()) {
        stateInitialize();
        return;
    }
    else if (phase == INIT)
    {
        state = BACKUP;
        emit(vrStateSignal, BACKUP);
        vf->setDisable();
        cancelPreemDelayTimer();
        stateBackup(TIMER_START);
    }
    else if (phase == TIMER_START)
    {
        scheduleMasterDownTimer();
    }
    else if (phase == TIMER_END)
    {
        vf->setEnable();
        sendAdvertisement();
        scheduleBroadcastTimer();
        //sendBroadcast();
        adverTimerInit = getAdvertisementInterval();

        EV << hostname << "# " << debugStateMachine("Backup", "Master") << endl;
        stateMaster(INIT);
    }
    else if (phase == STOP)
    {
        EV << hostname << "# " << debugStateMachine("Backup", "Initialize") << endl;
        cancelMasterDownTimer();
    }
}

void VRRPv2VirtualRouter::stateMaster(VRRPPhase phase)
{
    //Check whether interface is disconnected
    //IF yes THEN transit to INIT state
    if (!ie->isUp()) {
        stateInitialize();
        return;
    }
    else if (phase == INIT)
    {
        state = MASTER;
        advertisementIntervalActual = advertisementInterval;
        emit(vrStateSignal, MASTER);
        vf->setEnable();
        stateMaster(TIMER_START);
    }
    else if (phase == TIMER_START)
    {
        scheduleAdverTimer();
    }
    else if (phase == TIMER_END)
    {
        sendAdvertisement();
        adverTimerInit = getAdvertisementInterval();
        stateMaster(TIMER_START);
    }
    else if (phase == STOP)
    {
        cancelAdverTimer();
        priority = 0;
        sendAdvertisement();
        EV << hostname << "# " << debugStateMachine("Master", "Initialize") << endl;
    }
}

/***
 *    TIMERS
 */
void VRRPv2VirtualRouter::handleTimer(cMessage* msg)
{
    VRRPTimer category = (enum VRRPTimer) msg->getKind();

    if (category == ADVERTISE)
    {
        stateMaster(TIMER_END);
    }
    else if (category == MASTERDOWN && !preemtionDelay)
    {
        stateBackup(TIMER_END);
    }
    else if (category == BROADCAST)
    {
        sendBroadcast();
    }
    else if (category == PREEMTION)
    {
        stateBackup(TIMER_END);
    }
    else if (category == INITCHECK) {
        stateInitialize();
    }
}

void VRRPv2VirtualRouter::schedulePreemDelayTimer()
{
    if (preemtionTimer == NULL)
    {
        preemtionTimer = new cMessage(PREEMPTDLY_MSG, PREEMTION);
        scheduleAt(simTime() + preemTimerInit, preemtionTimer);
    }
}

void VRRPv2VirtualRouter::scheduleBroadcastTimer()
{
    cancelBroadcastTimer();
    broadcastTimer = new cMessage(BCASTDLY_MSG, BROADCAST);
    scheduleAt(simTime() + 0.0000001, broadcastTimer);
}

void VRRPv2VirtualRouter::scheduleAdverTimer()
{
    cancelAdverTimer();
    adverTimer = new cMessage(ADVTIMER_MSG, ADVERTISE);
    scheduleAt(simTime() + adverTimerInit, adverTimer);
}

void VRRPv2VirtualRouter::scheduleMasterDownTimer()
{
    cancelMasterDownTimer();
    masterDownTimer = new cMessage(MASTTIMER_MSG, MASTERDOWN);
    scheduleAt(simTime() + masterDownTimerInit, masterDownTimer);
}

void VRRPv2VirtualRouter::cancelTimer(cMessage* timer)
{
    if (timer != NULL)
    {
        cancelEvent(timer);
        delete timer;
    }
}

void VRRPv2VirtualRouter::cancelPreemDelayTimer()
{
    cancelTimer(preemtionTimer);
    preemtionTimer = NULL;
}

void VRRPv2VirtualRouter::cancelAdverTimer()
{
    cancelTimer(adverTimer);
    adverTimer = NULL;
}

void VRRPv2VirtualRouter::cancelMasterDownTimer()
{
    cancelTimer(masterDownTimer);
    masterDownTimer = NULL;
}

void VRRPv2VirtualRouter::cancelBroadcastTimer()
{
    cancelTimer(broadcastTimer);
    broadcastTimer = NULL;
}

/***
 *   MESSAGE
 */
void VRRPv2VirtualRouter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleTimer(msg);
        return;
    }
    else if (dynamic_cast<VRRPv2Advertisement*>(msg))
    {
        VRRPv2Advertisement* adver = dynamic_cast<VRRPv2Advertisement *>(msg);

        if (!handleAdvertisement(adver))
        {
            EV << " Discard it." << endl;
            delete msg;
            return;
        }

        EV << hostname << "# " << debugPacketBackup((int) adver->getPriority(), adver->getAddresses(0)) << endl;

        if (state == BACKUP)
            handleAdvertisementBackup(adver);

        else if (state == MASTER)
            handleAdvertisementMaster(adver);

        emit(recvAdverSignal, 1L);
    }
    //TODO Shutdown
    else
    {
        EV << "Unknown packet, discard it." << endl;
    }
    delete msg;
}

bool VRRPv2VirtualRouter::handleAdvertisement(VRRPv2Advertisement* msg)
{
    // MUST verify the TTL is 255
    IPv4ControlInfo* controlInfo = check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());

    if (controlInfo->getTimeToLive() != 255)
    {
        EV << "Invalid TTL. " << msg->getVersion() << " expect 255.";
        return false;
    }

    // MUST verify the VRRP version
    if (msg->getVersion() != version)
    {
        EV << "Invalid version. " << msg->getVersion() << " expect " << version << ".";
        return false;
    }

    // MUST verify that the received packet length is great that or equal to the VRRP header
    //TODO

    // MUST verify the VRRP checksum
    //TODO

    // Verifiy that the IP address(es) associeated with the VRID are valid.
    if (msg->getAddressesArraySize() == IPsecondary.size())
    {
        for (int i = 0; i < (int) msg->getAddressesArraySize(); i++)
            if (msg->getAddresses(i) != IPsecondary.at(i))
            {
                EV << "Invalid addresses." << " Bad address " << msg->getAddresses(i) << endl;
                return false;
            }
    }
    else
    {
        EV << "Invalid addresses." << "" << msg->getAddressesArraySize() << " " << IPsecondary.size() << endl;
    }

    // MUST verify that the Adver Interval in the packet is the same the locally configured for this virtual router
    if (advertisementIntervalActual != msg->getAdverInt() && !learn)
    {
        EV << "Advertisement interval mismatch mine="
                << advertisementIntervalActual << " received "
                << msg->getAdverInt() << "." << endl;

        return false;
    }

    return true;
}

void VRRPv2VirtualRouter::handleAdvertisementBackup(VRRPv2Advertisement* msg)
{
    if (((int) msg->getPriority()) == 0)
    {
        masterDownTimerInit = getSkewTime();
        stateBackup(TIMER_START);
        return;
    }
    else if (preemtion == false || ((int) msg->getPriority()) > priority)
    {
        masterDownTimerInit = getMasterDownInterval();
        stateBackup(TIMER_START);
    }
    else if (((int) msg->getPriority()) <= priority)
    {
        //Remain Backup
        if (msg->getPriority() == priority
                && ((IPv4ControlInfo *) msg->getControlInfo())->getSrcAddr() > ie->ipv4Data()->getIPAddress()) {
            masterDownTimerInit = getMasterDownInterval();
            stateBackup(TIMER_START);
            return;
        }

        //Become new Master
        if (preemtionDelay)
            schedulePreemDelayTimer();
        //Let us expire MDI
        else {
            //...do nothing
        }
    }
}

void VRRPv2VirtualRouter::handleAdvertisementMaster(VRRPv2Advertisement* msg)
{
    if (((int) msg->getPriority()) == 0)
    {
        sendAdvertisement();
        adverTimerInit = getAdvertisementInterval();
    }
    else if ((((int) msg->getPriority()) > priority)
            || (msg->getPriority() == priority
                    && ((IPv4ControlInfo *) msg->getControlInfo())->getSrcAddr() > ie->ipv4Data()->getIPAddress()))
    {
        cancelAdverTimer();
        masterDownTimerInit = getMasterDownInterval();

        if (learn)
        {
            advertisementIntervalActual = msg->getAdverInt();
        }

        EV << hostname << "# " << debugEvent() << endl;
        EV << hostname << "# " << debugStateMachine("Master", "Backup") << endl;
        stateBackup(INIT);
        return;
    }
    // else DISCARD Advertisement

    stateMaster(TIMER_START);
}

void VRRPv2VirtualRouter::sendAdvertisement() {

    IPv4ControlInfo* controlInfo = new IPv4ControlInfo();
    //TODO: Vesely - Is it correct to comment?
    //controlInfo->setMacSrc(virtualMAC);
    controlInfo->setSrcAddr(ie->ipv4Data()->getIPAddress());
    controlInfo->setDestAddr(multicastIP);
    controlInfo->setProtocol(protocol);
    controlInfo->setTimeToLive(ttl + 1);
    controlInfo->setInterfaceId(ie->getInterfaceId());

    VRRPv2Advertisement* msg = new VRRPv2Advertisement(VRRPADV_MSG);
    msg->setControlInfo(controlInfo);
    msg->setVersion(version);
    msg->setVrid(vrid);
    msg->setPriority(priority);
    msg->setCountIPAddrs(IPsecondary.size());
    msg->setAdverInt(advertisementIntervalActual);
    msg->setChecksum(getAdvertisementChecksum(version, vrid, priority, advertisementIntervalActual, IPsecondary));

    msg->setAddressesArraySize(IPsecondary.size());
    for (unsigned int i = 0; i < IPsecondary.size(); i++)
        msg->setAddresses(i, IPsecondary.at(i));

    EV << hostname << "# " << debugPacketMaster(msg->getChecksum()) << endl;
    send(msg, IPOUT_GATE);
    emit(sentAdverSignal, 1L);
};

void VRRPv2VirtualRouter::sendBroadcast()
{
    for (unsigned int i = 0; i < IPsecondary.size(); i++)
        arp->sendARPGratuitous(ie, virtualMAC, IPsecondary.at(i), arpType);

    emit(sentARPSignal, 1L);
};

/***
 *   UTILS
 */
void VRRPv2VirtualRouter::loadDefaultConfig()
{
    own = false;
    preemtionDelay = false;
    hostname = par(HOSTNAME_PAR);
    vrid = par(VRID_PAR);
    version = par("version");
    priority = par("priorityDefault");
    ttl = par("timeToLive");
    protocol = par("ipProtocol");
    preemtion = par("preemptDefault");
    learn = par("learnDefault");
    advertisementInterval = par("adverDefault");
    advertisementIntervalActual = par("adverDefault");
    const char * ip = par("multicastIPv4");
    multicastIP.set(ip);
    setArp((int) par("arpType"));
    arpDelay = par("arpDelay");
}

void VRRPv2VirtualRouter::setOwn() {
    if (IPprimary == ie->ipv4Data()->getIPAddress())
    {
        own = true;
        priority = (int) par(PRIOOWN_PAR);
    }
    else
    {
        own = false;
    }
}

std::string VRRPv2VirtualRouter::getStrOfState(VRRPState state) const
{
    if (state == INITIALIZE)
        return "Initialize";
    else if (state == BACKUP)
        return "Backup";
    else if (state == MASTER)
        return "Master";
    else
        return "Shutdown";
}

void VRRPv2VirtualRouter::createVirtualMAC()
{
    virtualMAC.setAddress(par("virtualMAC"));
    virtualMAC.setAddressByte(5, vrid);
}

uint16_t VRRPv2VirtualRouter::getAdvertisementChecksum(int version, int vrid, int priority, int advert, std::vector<IPv4Address> address) {

    uint32_t vrrpHead1 = 0;
    vrrpHead1 = version;

    // type
    vrrpHead1 = vrrpHead1 << 4 & 0xFFFFFFFF;
    vrrpHead1 += 1;

    // vrid
    vrrpHead1 = vrrpHead1 << 8 & 0xFFFFFFFF;
    vrrpHead1 += vrid;

    // priority
    vrrpHead1 = vrrpHead1 << 8 & 0xFFFFFFFF;
    vrrpHead1 += priority;

    // count
    vrrpHead1 = vrrpHead1 << 8 & 0xFFFFFFFF;
    vrrpHead1 += (int) address.size();

    uint32_t vrrpHead2 = 0;
    vrrpHead2 = advert << 16 & 0xFFFFFFFF;

    std::vector<uint32_t> data;
    data.push_back(vrrpHead1);
    data.push_back(vrrpHead2);
    for (int i = 0; i < (int) address.size(); i++)
        data.push_back(address.at(i).getInt());

    return serializer::TCPIPchecksum::checksum(data.data(), data.size() * 4);
}
/***
 *   PRINT
 */
std::string VRRPv2VirtualRouter::info() const
{
    return description + " (" +cSimpleModule::info() + ")";
}

std::string VRRPv2VirtualRouter::detailedInfo() const
{
    return "detailedInfo";
}

std::string VRRPv2VirtualRouter::showBrief() const
{
    std::stringstream result;
    result << "Iface:" << ie->getName();
    result << " Grp:" << vrid;
    result << " Pri:" << priority;
    result << " Own:" << this->getStrOfBool(own);
    result << " Pre:" << this->getStrOfBool(preemtion);
    result << " State:" << this->getStrOfState(state);
    //TODO result << " MasterAddr:
    result << " GroupAddr:" << IPprimary;

    return result.str();
}

std::string VRRPv2VirtualRouter::showConfig() const
{
    std::stringstream result;
    result << "IPAddress:" << IPprimary;

    if (IPsecondary.size() > 0)
    {
        result << " IPSecondary:{";
        for (int i = 1; i < (int) IPsecondary.size(); i++)
            result << IPsecondary.at(i) << ((i != (int) IPsecondary.size() - 1) ? "," : "");
        result << "}";
    }

    if (!description.empty())
        result << " Description" << "{" << description << "}";
    result << " Priority:" << priority;
    result << " Preempt:" << this->getStrOfBool(preemtion);
    result << " TimerAdvertise:" << advertisementIntervalActual;
    result << " TimerLearn:" << this->getStrOfBool(learn);

    return result.str();
}

/***
 *   DEBUG MESSAGE
 */
std::string VRRPv2VirtualRouter::debugStateMachine(std::string from, std::string to) const
{
    std::stringstream result;
    result << "%VRRP-6-STATECHANGE: " << ie->getName() << " Grp" << vrid << " state " << from << " -> " << to;
    return result.str();
}

std::string VRRPv2VirtualRouter::debugPacketMaster(uint16_t checksum) const
{
    std::stringstream result;
    result << "Grp " << vrid << " sending Advertisement checksum 0x" << std::hex << checksum;
    return result.str();
}

std::string VRRPv2VirtualRouter::debugPacketBackup(int priority, IPv4Address ipaddr) const
{
    std::stringstream result;
    result << "Grp " << vrid << " Advertisement priority " << priority << ", ipaddr " << ipaddr;
    return result.str();
}

std::string VRRPv2VirtualRouter::debugEvent() const
{
    std::stringstream result;
    result << "Grp " << vrid << " Event - Advert higher or equal priority";
    return result.str();
}

void VRRPv2VirtualRouter::scheduleInitCheck() {
    if (initCheckTimer == NULL)
        initCheckTimer = new cMessage(INITCHCK_MSG, INITCHECK);
    scheduleAt(simTime() + (double)advertisementInterval/4, initCheckTimer);
}

}
