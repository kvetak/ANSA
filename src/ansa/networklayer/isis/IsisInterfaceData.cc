// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file IsisInterfaceData.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 1.4.2013
 * @brief Stores IS-IS interface related data.
 * @detail Stores IS-IS interface related data.
 * @todo Z9
 */

#include "ansa/networklayer/isis/IsisInterfaceData.h"

namespace inet {


IsisInterfaceData::IsisInterfaceData()
{
    // TODO Auto-generated constructor stub

    this->helloValid = false;






    ifaceId = -1;
    gateIndex = -1;
    network = ISIS_NETWORK_BROADCAST; //previously broadcast = true
    passive = false;
    isisEnabled = true; /*!<is IS-IS activated on this interface? (default yes for all ifts)*/
    circuitType = ISIS_CIRCUIT_L1;
    priority = -1; /*!<interface priority for being designated IS*/
    l1DISPriority = 0; /*!<priority of current L1 DIS*/
    l2DISPriority = 0; /*!<priority of currend L2 DIS*/
    PseudonodeID l1DIS;
    PseudonodeID l2DIS;
    metric = 0; /*!<interface metric (default 10)*/
    L1HelloInterval = -1; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    L2HelloInterval = -1; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    L1HelloMultiplier = -1; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
    L2HelloMultiplier = -1; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */
    lspInterval = -1; /*!< Minimum delay in ms between sending two successive LSPs.*/
    l1CsnpInterval = -1; /*!< Interval in seconds between generating CSNP message.*/
    l2CsnpInterval = -1; /*!< Interval in seconds between generating CSNP message.*/
    l1PsnpInterval = -1; /*!< Interval in seconds between generating PSNP message.*/
    l2PsnpInterval = -1; /*!< Interval in seconds between generating PSNP message.*/
}

void IsisInterfaceData::init(void){


}

Ptr<ISISMessage> IsisInterfaceData::getHello()
{
    return hellos.front();
}

std::vector<Ptr<ISISMessage>> IsisInterfaceData::getHellos()
{
    return hellos;
}

void IsisInterfaceData::setHello(Ptr<ISISMessage> hello)
{
    this->hellos.clear();
    this->hellos.push_back(hello);

}

void IsisInterfaceData::clearHello(void){
    for(std::vector<Ptr<ISISMessage>>::iterator it = this->hellos.begin(); it != this->hellos.end();){
        //TODO ANSAINET4.0 Uncomment
//        delete (*it);
        it = this->hellos.erase(it);
    }
//    this->hellos.clear();
}

void IsisInterfaceData::addHello(Ptr<ISISMessage> hello){
    this->hellos.push_back(hello);
}

bool IsisInterfaceData::isHelloValid() const
{
    return helloValid;
}

ISISCircuitType IsisInterfaceData::getCircuitType() const
{
    return circuitType;
}

void IsisInterfaceData::setCircuitType(ISISCircuitType circuitType)
{
    if(this->circuitType != circuitType){
        this->setHelloValid(false);
    }
    this->circuitType = circuitType;
}

int IsisInterfaceData::getGateIndex() const
{
    return gateIndex;
}

void IsisInterfaceData::setGateIndex(int gateIndex)
{
    if(this->gateIndex != gateIndex){
        this->setHelloValid(false);
    }
    this->gateIndex = gateIndex;
}

int IsisInterfaceData::getIfaceId() const
{
    return ifaceId;
}

void IsisInterfaceData::setIfaceId(int ifaceId)
{
    if(this->ifaceId != ifaceId){
        this->setHelloValid(false);
    }
    this->ifaceId = ifaceId;
}

bool IsisInterfaceData::isIsisEnabled() const
{
    return isisEnabled;
}

void IsisInterfaceData::setIsisEnabled(bool isisEnabled)
{
    if(this->isisEnabled != isisEnabled){
        this->setHelloValid(false);
    }
    this->isisEnabled = isisEnabled;
}

int IsisInterfaceData::getL1CsnpInterval() const
{
    return l1CsnpInterval;
}

void IsisInterfaceData::setL1CsnpInterval(int l1CsnpInterval)
{

    this->l1CsnpInterval = l1CsnpInterval;
}

const PseudonodeID IsisInterfaceData::getL1Dis() const
{
    return l1DIS;
}

void IsisInterfaceData::setL1Dis(const PseudonodeID& l1DIS)
{
  this->l1DIS = PseudonodeID(l1DIS);
//    memcpy(this->l1DIS, l1DIS, ISIS_LAN_ID);
}

unsigned int IsisInterfaceData::getL1DisPriority() const
{
    return l1DISPriority;
}

void IsisInterfaceData::setL1DisPriority(unsigned int l1DisPriority)
{
    l1DISPriority = l1DisPriority;
}

int IsisInterfaceData::getL1HelloInterval() const
{
    return L1HelloInterval;
}

void IsisInterfaceData::setL1HelloInterval(int l1HelloInterval)
{
    if(this->L1HelloInterval != l1HelloInterval){
        this->setHelloValid(false);
    }
    L1HelloInterval = l1HelloInterval;
}

short IsisInterfaceData::getL1HelloMultiplier() const
{

    return L1HelloMultiplier;
}

void IsisInterfaceData::setL1HelloMultiplier(short l1HelloMultiplier)
{
    if(this->L1HelloMultiplier != l1HelloMultiplier){
            this->setHelloValid(false);
        }
    L1HelloMultiplier = l1HelloMultiplier;
}

int IsisInterfaceData::getL1PsnpInterval() const
{
    return l1PsnpInterval;
}

void IsisInterfaceData::setL1PsnpInterval(int l1PsnpInterval)
{
    this->l1PsnpInterval = l1PsnpInterval;
}

int IsisInterfaceData::getL2CsnpInterval() const
{
    return l2CsnpInterval;
}

void IsisInterfaceData::setL2CsnpInterval(int l2CsnpInterval)
{
    this->l2CsnpInterval = l2CsnpInterval;
}

const PseudonodeID IsisInterfaceData::getL2Dis() const
{
    return l2DIS;
}

void IsisInterfaceData::setL2Dis(const PseudonodeID& l2DIS)
{
  this->l2DIS = PseudonodeID(l2DIS);
//    memcpy(this->l2DIS, l2DIS, ISIS_LAN_ID);
}

unsigned int IsisInterfaceData::getL2DisPriority() const
{
    return l2DISPriority;
}

void IsisInterfaceData::setL2DisPriority(unsigned int l2DisPriority)
{
    l2DISPriority = l2DisPriority;
}

int IsisInterfaceData::getL2HelloInterval() const
{
    return L2HelloInterval;
}

void IsisInterfaceData::setL2HelloInterval(int l2HelloInterval)
{
    if(this->L2HelloInterval != l2HelloInterval){
            this->setHelloValid(false);
        }
    L2HelloInterval = l2HelloInterval;
}

short IsisInterfaceData::getL2HelloMultiplier() const
{
    return L2HelloMultiplier;
}

void IsisInterfaceData::setL2HelloMultiplier(short l2HelloMultiplier)
{
    if(this->L2HelloMultiplier != l2HelloMultiplier){
            this->setHelloValid(false);
        }
    L2HelloMultiplier = l2HelloMultiplier;
}

int IsisInterfaceData::getL2PsnpInterval() const
{
    return l2PsnpInterval;
}

void IsisInterfaceData::setL2PsnpInterval(int l2PsnpInterval)
{
    this->l2PsnpInterval = l2PsnpInterval;
}

int IsisInterfaceData::getLspInterval() const
{
    return lspInterval;
}

void IsisInterfaceData::setLspInterval(int lspInterval)
{
    this->lspInterval = lspInterval;
}

unsigned int IsisInterfaceData::getMetric() const
{
    return metric;
}

void IsisInterfaceData::setMetric(unsigned int metric)
{
    this->metric = metric;
}

ISISNetworkType IsisInterfaceData::getNetwork() const
{
    return network;
}

void IsisInterfaceData::setNetwork(ISISNetworkType network)
{
    if(this->network != network){
            this->setHelloValid(false);
        }
    this->network = network;
}

bool IsisInterfaceData::isPassive() const
{
    return passive;
}

void IsisInterfaceData::setPassive(bool passive)
{
    if(this->passive != passive){
            this->setHelloValid(false);
        }
    this->passive = passive;
}

unsigned int IsisInterfaceData::getPriority() const
{
    return priority;
}

void IsisInterfaceData::setPriority(unsigned int priority)
{
    if(this->priority != priority){
            this->setHelloValid(false);
        }
    this->priority = priority;
}

void IsisInterfaceData::setHelloValid(bool helloValid)
{
    this->helloValid = helloValid;
}

IsisInterfaceData::~IsisInterfaceData()
{
    // TODO Auto-generated destructor stub
}

}//end namespace inet
