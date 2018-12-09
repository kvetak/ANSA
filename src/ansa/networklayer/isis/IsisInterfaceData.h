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
 * @file IsisInterfaceData.h
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 1.4.2013
 * @brief Stores IS-IS interface related data.
 * @detail Stores IS-IS interface related data.
 * @todo Z9
 */

#ifndef ISISINTERFACEDATA_H_
#define ISISINTERFACEDATA_H_

#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/networklayer/isis/ISISMessage_m.h"
#include "ansa/networklayer/isis/ISIStypes.h"

namespace inet {

class IsisInterfaceData : public InterfaceProtocolData
{


    private:
        //TODO for normal ISIS, there needsto be two sets of Hellos (L1 L2)
        std::vector<ISISMessage *> hellos;
        bool helloValid; // mark the Hello message invalid so during next Hello send it needs to be re-generated

        /* From ISISinterface */
        int ifaceId = -1; //previously intID
        int gateIndex = -1;
        ISISNetworkType network; //previously broadcast = true
        bool passive;
        bool isisEnabled; /*!<is IS-IS activated on this interface? (default yes for all ifts)*/
        ISISCircuitType circuitType;
        unsigned int priority; /*!<interface priority for being designated IS*/
        unsigned int l1DISPriority; /*!<priority of current L1 DIS*/
        unsigned int l2DISPriority; /*!<priority of currend L2 DIS*/
//        unsigned char l1DIS[ISIS_LAN_ID]; /*!<L1 designated router ID for ift*/
        PseudonodeID l1DIS;
//        unsigned char l2DIS[ISIS_LAN_ID]; /*!<L2 designated router ID for ift*/
        PseudonodeID l2DIS;
        unsigned int metric; /*!<interface metric (default 10)*/
        int L1HelloInterval; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
        int L2HelloInterval; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
        short L1HelloMultiplier; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
        short L2HelloMultiplier; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */
        int lspInterval; /*!< Minimum delay in ms between sending two successive LSPs.*/
        int l1CsnpInterval; /*!< Interval in seconds between generating CSNP message.*/
        int l2CsnpInterval; /*!< Interval in seconds between generating CSNP message.*/
        int l1PsnpInterval; /*!< Interval in seconds between generating PSNP message.*/
        int l2PsnpInterval; /*!< Interval in seconds between generating PSNP message.*/


    public:
        IsisInterfaceData();
        virtual ~IsisInterfaceData();
        void init(void);
        ISISMessage* getHello();
        std::vector<ISISMessage *> getHellos();
        void setHello(ISISMessage *hello);
        void clearHello(void);
        void addHello(ISISMessage *hello);
        bool isHelloValid() const;
        void setHelloValid(bool helloValid);
        ISISCircuitType getCircuitType() const;
        void setCircuitType(ISISCircuitType circuitType);
        int getGateIndex() const;
        void setGateIndex(int gateIndex);
        int getIfaceId() const;
        void setIfaceId(int ifaceId);
        bool isIsisEnabled() const;
        void setIsisEnabled(bool isisEnabled);
        int getL1CsnpInterval() const;
        void setL1CsnpInterval(int l1CsnpInterval);
        const PseudonodeID getL1Dis() const;
        void setL1Dis(const PseudonodeID& l1DIS);
        unsigned int getL1DisPriority() const;
        void setL1DisPriority(unsigned int l1DisPriority);
        int getL1HelloInterval() const;
        void setL1HelloInterval(int l1HelloInterval);
        short getL1HelloMultiplier() const;
        void setL1HelloMultiplier(short l1HelloMultiplier);
        int getL1PsnpInterval() const;
        void setL1PsnpInterval(int l1PsnpInterval);
        int getL2CsnpInterval() const;
        void setL2CsnpInterval(int l2CsnpInterval);
        const PseudonodeID getL2Dis() const;
        void setL2Dis(const PseudonodeID& l2DIS);
        unsigned int getL2DisPriority() const;
        void setL2DisPriority(unsigned int l2DisPriority);
        int getL2HelloInterval() const;
        void setL2HelloInterval(int l2HelloInterval);
        short getL2HelloMultiplier() const;
        void setL2HelloMultiplier(short l2HelloMultiplier);
        int getL2PsnpInterval() const;
        void setL2PsnpInterval(int l2PsnpInterval);
        int getLspInterval() const;
        void setLspInterval(int lspInterval);
        unsigned int getMetric() const;
        void setMetric(unsigned int metric);
        ISISNetworkType getNetwork() const;
        void setNetwork(ISISNetworkType network);
        bool isPassive() const;
        void setPassive(bool passive);
        unsigned int getPriority() const;
        void setPriority(unsigned int priority);
};

}//end namespace inet

#endif /* ISISINTERFACEDATA_H_ */
