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
 * @file TrillInterfaceData.h
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 24.3.2013
 * @brief Represents TRILL related data on interface.
 * @detail Represents TRILL related data on interface.
 * @todo Z9
 */

#ifndef TRILLINTERFACEDATA_H_
#define TRILLINTERFACEDATA_H_


#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/linklayer/rbridge/TRILLCommon.h"

namespace inet {

class TrillInterfaceData : public InterfaceProtocolData
{
    public:
        //        typedef enum{
        //            RBP_LOOPBACK = -1, //loopback not physical interface
        //            RBP_PORT,
        //            RBP_VLAN, //not implemented - intented to easily represent VLAN and all associated interfaces (gates)
        //        } RBPType;

        typedef std::vector<int> VLANVector;
        TrillInterfaceData();
        virtual ~TrillInterfaceData();
        void setDefaults(void);
        int getVlanId() const;
        bool isAccess() const;
        bool isDisLearning() const;
        bool isDisabled() const;
        bool isP2p() const;
        bool isTrunk() const;
        void setAccess(bool access);
        void setDisLearning(bool disLearning);
        void setDisabled(bool disabled);
        void setP2p(bool p2p);
        void setTrunk(bool trunk);
        void setVlanId(int vlanId);
        bool isAppointedForwarder(int vlanId, TRILLNickname nickname); //port is gateIndex
        bool isEnabled(int vlanId);
        bool isInhibited() const;
        void setInhibited(bool inhibited);
        int getDesigVlan() const;
        void setDesigVlan(int desigVlan);
        bool isVlanMapping() const;
        void setVlanMapping(bool vlanMapping);
        int getDesiredDesigVlan() const;
        void setDesiredDesigVlan(int desiredDesigVlan);
        void setAppointedForwarder(int vlanId, TRILLNickname nickname);
        void addAppointedForwarder(int vlanId, TRILLNickname nickname);
        void removeAppointedFowrwarder(int vlanId, TRILLNickname nickname);
        void clearAppointedForwarder();

    private:
        int interfaceId;
        int gateIndex;
//    RBPType type;
        //RFC 6325 4.9
        bool disabled; //disable bit defaults to enable (false)
        bool trunk; //end station service disabled a.k.a trunk port (default false)
        bool access; //trill traffic disabled on access port (default false)
        bool p2p; //use point to point hellos default false
        //Dominance relationship is as follows: Disable > P2P > Access > Trunk
        VLANVector announcingSet; //announcing VLAN set default to enabled VLANs
        VLANVector enabledgSet; //enabled VLAN set
        bool nonAdj; //accepting TRILL frames from non IS-IS adjacency port
        int inhibitionInterval; //0-30s
        bool disLearning; //disable learning (false means learning enabled)
        int vlanId;
        bool inhibited;
        int desigVLAN; //designated VLAN being used
        int desiredDesigVLAN; //desired Designated VLAN for this port this RBridge becomes DRB (DIS)
        bool vlanMapping;

//        VLANVector appointedForwarder;

        std::map<int, TRILLNickname> appointedForwarder; //map<vlanId, nickname>


};

}

#endif /* TRILLINTERFACEDATA_H_ */
