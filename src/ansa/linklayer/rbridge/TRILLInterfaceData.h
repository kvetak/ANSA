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

#ifndef TRILLINTERFACEDATA_H_
#define TRILLINTERFACEDATA_H_

#include "InterfaceEntry.h"

class TRILLInterfaceData : public InterfaceProtocolData
{
public:
    //        typedef enum{
    //            RBP_LOOPBACK = -1, //loopback not physical interface
    //            RBP_PORT,
    //            RBP_VLAN, //not implemented - intented to easily represent VLAN and all associated interfaces (gates)
    //        } RBPType;

        typedef std::vector<int> VLANVector;
    TRILLInterfaceData();
    virtual ~TRILLInterfaceData();
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
    bool isAppointedForwarder(int vlanId); //port is gateIndex
    bool isInhibited() const;
    void setInhibited(bool inhibited);
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

    VLANVector appointedForwarder;


};

#endif /* TRILLINTERFACEDATA_H_ */
