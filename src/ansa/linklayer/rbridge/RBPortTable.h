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
 * @file RBPortTable.h
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 20.3.2013
 * @brief
 * @detail
 * @todo
 */



#ifndef RBPORTTABLE_H_
#define RBPORTTABLE_H_

#include <csimplemodule.h>
#include <vector>

class RBPort
{
        typedef enum{
            RBP_LOOPBACK = -1, //loopback not physical interface
            RBP_PORT,
            RBP_VLAN, //not implemented - intented to easily represent VLAN and all associated interfaces (gates)
        } RBPType;
private:
    int interfaceId;
    int gateIndex;
    RBPType type;
    //RFC 6325 4.9
    bool disabled; //disable bit defaults to enable (false)
    bool trunk; //end station service disabled a.k.a trunk port (default false)
    bool access; //trill trafic disabled on access port (default false)
    bool p2p; //use point to point hellos default false
    //Dominance relationship is as follows: Disable > P2P > Access > Trunk
    std::vector<int> announcingSet; //announcing VLAN set default to enabled VLANs
    std::vector<int> enabledgSet; //enabled VLAN set
    bool nonAdj; //accepting TRILL frames from non IS-IS adjacency port
    int inhibitionInterval; //0-30s
    bool learning; //disable learning

public:
    void setInterfaceId(int interfaceId);
    void setType(RBPType type);
    void initDefaults();
    int getGateIndex() const;
    int getInterfaceId() const;
    RBPType getType() const;
    void setGateIndex(int gateIndex);
};



class RBPortTable : public cSimpleModule
{
    public:
        typedef enum {
                    OFF = 0,
                    BLOCKING = 1,
                    DISCARTING = 2,
                    LEARNING = 3,
                    FORWARDING = 4,
                } tRBPortState;
            RBPortTable();
            virtual ~RBPortTable();
            RBPort* getPortByGateIndex(int gateIndex) ;

    private:
        typedef std::vector<RBPort> tRBPortTable;

        tRBPortTable portTable;

        void initDefaults(void);

    protected:
        virtual void initialize(int stage);
        virtual int numInitStages() const
        {
            return 5;
        }
        virtual void handleMessage(cMessage *msg){
            throw cRuntimeError("handleMessage doesn't implemented in RBPortTable");
        }
};

#endif /* RBPORTTABLE_H_ */
