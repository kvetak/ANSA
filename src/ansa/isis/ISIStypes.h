// Author: Matej Hrncirik
// FIT VUT 2012
//
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

#ifndef ISISTYPES_H_
#define ISISTYPES_H_

#include "ISISTimer_m.h"

/**
 * Structure for storing info about all active interfaces.
 */
struct ISISinterface
{
    int intID;                /*!<interface ID*/
    int gateIndex;            /*!<gate index*/
    //bool broadcast;         /*!<broadcast enabled?*/
    //bool loopback;          /*!<is it loopback?*/
    bool passive;             /*!<is it passive intf?*/
    bool ISISenabled;         /*!<is IS-IS activated on this interface? (default yes for all ifts)*/
    short circuitType;        /*!<circuit type  L1, L2, L1L2*/
    unsigned char priority;   /*!<interface priority for being designated IS*/
    unsigned char L1DISpriority;    /*!<priority of current L1 DIS*/
    unsigned char L2DISpriority;    /*!<priority of currend L2 DIS*/
    unsigned char L1DIS[7];   /*!<L1 designated router ID for ift*/
    unsigned char L2DIS[7];   /*!<L2 designated router ID for ift*/
    unsigned char metric;     /*!<interface metric (default 10)*/
    InterfaceEntry *entry;    /*!< other interface info*/
};

/**
 * Structure for storing info about neighbours
 */
struct ISISadj
{
    unsigned char sysID[6];             /*!<system ID of neighbour*/
    unsigned char areaID[3];            /*!<neighbour areaID*/
    MACAddress mac;                     /*!<mac address of neighbour*/
    bool state;                         /*!<adjacency state has to be 2-way; 0 = only 1 way, 1 = 2-way (hello received from adj router)*/
    ISISTimer *timer;                   /*!<timer set to hold time and reseted every time hello from neighbour is received*/
    int gateIndex;                      /*!<index of gate, which is neighbour connected to*/
};

/**
 * Strucure for storing of narrow metrics of interface
 */
struct metrics_t
{
    unsigned char defaultMetric;  /*!<Default metric (only this one is used)*/
    unsigned char delayMetric;    /*!<not used*/
    unsigned char expenseMetric;  /*!<not used*/
    unsigned char errortMetric;   /*!<not used*/
};

/**
 * Structure representing content of entry in link-state db: LAN ID associated with metrics
 */
struct LSPneighbour
{
    unsigned char LANid[7]; /*!LAN ID = System ID (6B)+ Pseudonode ID (1B)*/
    metrics_t metrics;      /*!metric*/

};

struct LSPrecord
{
    std::vector<LSPneighbour> neighbours;    //list of neighbours
    unsigned char LSPid[8];             //ID of LSP
    unsigned long seq;                  //sequence number
    ISISTimer *deadTimer;               //dead timer - 1200s
};




#endif /* ISISTYPES_H_ */
