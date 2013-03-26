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
#include "ISISMessage_m.h"
//#include "ISIS.h"


#define ISIS_DIS_PRIORITY 64 /*!< Default priority to become DIS*/
#define ISIS_METRIC 10 /*!< Default "default" metric value*/
#define ISIS_HELLO_INTERVAL 10 /*!< Default hello interval in seconds*/
#define ISIS_HELLO_MULTIPLIER 3 /*!< Default hello multiplier value.*/
#define ISIS_SYSTEM_ID 6 /*!< Length of System-ID.*/
#define ISIS_MAX_AREAS 3 /*!< Default value for Maximum Areas.*/
#define ISIS_AREA_ID 3 /*!< Length of Area-ID.*/
#define ISIS_LSP_INTERVAL 33 /*!< Minimum delay in ms between sending two successive LSPs. */
#define ISIS_LSP_REFRESH_INTERVAL 150 /*!< Interval in seconds at which LSPs are refreshed (1 to 65535). This value SHOULD be less than ISIS_LSP_MAX_LIFETIME*/
#define ISIS_LSP_MAX_LIFETIME 200 /*!< Interval in seconds during which is specified LSP valid. This value SHOULD be more than ISIS_LSP_REFRESH_INTERVAL */
#define ISIS_LSP_GEN_INTERVAL 5 /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
#define ISIS_LSP_INIT_WAIT 50 /*!< Initial wait interval in ms before generating first LSP.*/
#define ISIS_CSNP_INTERVAL 10 /*!< Interval in seconds between generating CSNP message.*/
#define ISIS_PSNP_INTERVAL 2 /*!< Interval in seconds between generating CSNP message.*/
#define ISIS_LSP_MAX_SIZE 1492 /*!< Maximum size of LSP in Bytes.*/ //TODO change to something smaller so we can test it
#define ISIS_LSP_SEND_INTERVAL 5 /*!< Interval in seconds between periodic scanning LSP Database and checking SRM and SSN flags.*/
#define ISIS_SPF_FULL_INTERVAL 50
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
    unsigned char L1DIS[ISIS_SYSTEM_ID + 1];   /*!<L1 designated router ID for ift*/
    unsigned char L2DIS[ISIS_SYSTEM_ID + 1];   /*!<L2 designated router ID for ift*/
    unsigned char metric;     /*!<interface metric (default 10)*/
    int L1HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    int L2HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    short L1HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
    short L2HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */
    int lspInterval;                            /*!< Minimum delay in ms between sending two successive LSPs.*/
    int L1CsnpInterval;                           /*!< Interval in seconds between generating CSNP message.*/
    int L2CsnpInterval;                           /*!< Interval in seconds between generating CSNP message.*/
    int L1PsnpInterval;                           /*!< Interval in seconds between generating PSNP message.*/
    int L2PsnpInterval;                           /*!< Interval in seconds between generating PSNP message.*/

    InterfaceEntry *entry;    /*!< other interface info*/
};
typedef std::vector<ISISinterface> ISISInterTab_t;


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
    bool network;                       /*!<network type, true = broadcast, false = point-to-point*/
    ISISTimer *desVLANTimer;
    ISISTimer *nonDesVLANTimer;
    int priority; //priority of the neighbour to be the DRB
    int desiredDesVLAN; // neighbour's desired VLAN to the designated VLAN

    //bool operator for sorting
    bool operator<(const ISISadj& adj2) const {

        for (unsigned int j = 0; j < ISIS_AREA_ID; j++){
            if(areaID[j] < adj2.areaID[j]){
                return true; //first is smaller, so return true
            }else if(areaID[j] > adj2.areaID[j]){
                return false; //first is bigger, so return false
            }
            //if it's equal then continue to next one
        }
        //AreaIDs match, so compare system IDs

        for (unsigned int i = 0; i < ISIS_SYSTEM_ID; i++){
            if(sysID[i] < adj2.sysID[i]){
                return true; //first is smaller, so return true
            }else if(sysID[i] > adj2.sysID[i]){
                return false; //first is bigger, so return false
            }
            //if it's equal then continue to next one
        }

        //if the first MAC address is smaller, return true
        if(mac.compareTo(adj2.mac) < 0){
            return true;
        }

        //if they're equal, return false
        return false;
    }
};

typedef std::vector<ISISadj> AdjTab_t;

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

struct LSPRecord
{
        ISISLSPPacket* LSP; //link-state protocol data unit
        ISISTimer *deadTimer; //dead timer
        std::vector<bool> SRMflags;
        std::vector<bool> SSNflags;
        double simLifetime; /*!< specify deadTi */

/*        LSPRecord(){
            for (std::vector<ISISinterface>::iterator intIt = this->ISISIft.begin(); intIt != this->ISISIft.end(); ++intIt)
            {

                this->SRMflags.push_back(false);
                this->SSNflags.push_back(false);

            }
        }*/

        ~LSPRecord(){

/*            for (unsigned int i = 0; i < this->LSP->getTLVArraySize(); i++)
            {
                if(this->LSP->getTLV(i).value != NULL){
                    delete this->LSP->getTLV(i).value;
                }
            }*/
            this->LSP->setTLVArraySize(0);
            if(this->LSP != NULL){
                delete this->LSP;
            }
//            if(this->deadTimer != NULL){
//                drop(this->deadTimer);
//                delete this->deadTimer;
//            }
            this->SRMflags.clear();
            this->SSNflags.clear();
        }

        //bool operator for sorting
         bool operator<(const LSPRecord& lspRec2) const {

             for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++){
                 if(this->LSP->getLspID(i) < lspRec2.LSP->getLspID(i)){
                     return true; //first is smaller, so return true
                 }else if(this->LSP->getLspID(i) > lspRec2.LSP->getLspID(i)){
                     return false; //first is bigger, so return false
                 }
                 //if it's equal then continue to next one
             }

             //if they're equal, return false
             return false;
         }

};
typedef std::vector<LSPRecord *> LSPRecQ_t;

struct FlagRecord
{
        LSPRecord *lspRec;
        int index;
        //destructor!!
        ~FlagRecord(){

        }
};
typedef std::vector<FlagRecord*> FlagRecQ_t;
typedef std::vector<std::vector<FlagRecord*> *> FlagRecQQ_t;
struct ISISNeighbour
{
        unsigned char *id;
        //uint32_t metric;
        bool type; //should represent whether it's a leaf node; true = leaf
        InterfaceEntry *entry;

        ISISNeighbour(){

        }

        ISISNeighbour(unsigned char * id, bool type, InterfaceEntry *entry){
            this->id = new unsigned char [ISIS_SYSTEM_ID + 2];
            memcpy(this->id, id, ISIS_SYSTEM_ID + 2);
            this->type = false;
            this->entry = entry;
        }

        ~ISISNeighbour(){
            delete id;
        }

        ISISNeighbour *copy(){
            return new ISISNeighbour(this->id, this->type, this->entry);
        }

};

typedef std::vector<ISISNeighbour*> ISISNeighbours_t;

struct ISISPath
{
        unsigned char *to;
        uint32_t metric;
        ISISNeighbours_t from; // works as next hop
        //bool operator for sorting

        bool operator()(const ISISPath *path1, const ISISPath *path2) {

        //             for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++){
                if(path1->metric < path2->metric){
                    return true; //first is smaller, so return true
                }else if(path1->metric > path2->metric){
                    return false; //first is bigger, so return false
                }else{
                    if(path1->to[ISIS_SYSTEM_ID] > path2->to[ISIS_SYSTEM_ID]){
                        return true;
                    }
                }
                //if it's equal then continue to next one
        //             }

            //if they're equal, return false
            return false;
        }

};


typedef std::vector<ISISPath*> ISISPaths_t;


typedef std::vector<LSPRecord *> ISISLspDb_t;

struct ISISCon
{
        unsigned char *from;
        unsigned char *to;
        uint32_t metric;
        bool type;
        InterfaceEntry *entry;
};
typedef std::vector<ISISCon*> ISISCons_t;

/*
 * Adjacency states for 3-way handshake on point-to-point links
 */
enum PTP_HELLO_ADJ_STATE
{
    PTP_UP = 1,
    PTP_INIT = 2,
    PTP_DOWN = 3
};

/*
 *
 */






#endif /* ISISTYPES_H_ */
