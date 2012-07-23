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

#include "MACAddress.h"
#include "InterfaceEntry.h"
#include "ISISTimer_m.h"


#define ISIS_DIS_PRIORITY 64 /*!< Default priority to become DIS*/
#define ISIS_METRIC 10 /*!< Default "default" metric value*/
#define ISIS_HELLO_INTERVAL 10 /*!< Default hello interval in seconds*/
#define ISIS_HELLO_MULTIPLIER 3 /*!< Default hello multiplier value.*/
#define ISIS_SYSTEM_ID 6 /*!< Length of System-ID.*/
#define ISIS_MAX_AREAS 3 /*!< Default value for Maximum Areas.*/
#define ISIS_AREA_ID 3

//class InterfaceEntr;
//class MACAddress;

/**
 * Structure for storing info about all active interfaces.
 */
struct ISISinterface
{
    int intID;                /*!<interface ID*/
    int gateIndex;            /*!<gate index*/
    bool network;             /*!< network type, true = broadcast, false = point-to-point */
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
    int L1HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    int L2HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    short L1HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
    short L2HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */

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

/*
 * Adjacency states for 3-way handshake on point-to-point links
 */
enum PTP_HELLO_ADJ_STATE
{
    PTP_UP = 1,
    PTP_INIT = 2,
    PTP_DOWN = 3
};



#endif /* ISISTYPES_H_ */
