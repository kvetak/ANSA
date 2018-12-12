// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file ISIStypes.h
 * @author Matej Hrncirik, Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 7.3.2012
 * @brief Contains various types used in IS-IS module.
 * @detail Contains various types used in IS-IS module. Namely structure fo adjacency, LSP record and path with metric to its neighbors.
 * @todo
 */

#ifndef ISISTYPES_H_
#define ISISTYPES_H_

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/networklayer/isis/ISISTimer_m.h"
#include "ansa/networklayer/isis/ISISMessage_m.h"
//#include "ansa/networklayer/isis/ISISCommon.h"
//#include "ISIS.h"


namespace inet {

#define ISIS_DIS_PRIORITY 64 /*!< Default priority to become DIS*/
#define ISIS_METRIC 10 /*!< Default "default" metric value*/
#define ISIS_HELLO_INTERVAL 10 /*!< Default hello interval in seconds*/
#define ISIS_HELLO_MULTIPLIER 3 /*!< Default hello multiplier value.*/
//#define ISIS_SYSTEM_ID 6 /*!< Length of System-ID.*/
#define ISIS_LAN_ID ISIS_SYSTEM_ID + 1 /*!< Length of LAN-ID.*/
#define ISIS_MAX_AREAS 1 /*!< Default value for Maximum Areas.*/
#define ISIS_AREA_ID 3 /*!< Length of Area-ID.*/
#define ISIS_LSP_INTERVAL 33 /*!< Minimum delay in ms between sending two successive LSPs. */
#define ISIS_LSP_REFRESH_INTERVAL 150 /*!< Interval in seconds at which LSPs are refreshed (1 to 65535). This value SHOULD be less than ISIS_LSP_MAX_LIFETIME*/
#define ISIS_LSP_MAX_LIFETIME 200 /*!< Interval in seconds during which is specified LSP valid. This value SHOULD be more than ISIS_LSP_REFRESH_INTERVAL */
#define ISIS_LSP_GEN_INTERVAL 5 //TODO A! change back to 5 /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
#define ISIS_LSP_INIT_WAIT 50 //TODO A! change back to 50 /*!< Initial wait interval in ms before generating first LSP.*/
#define ISIS_CSNP_INTERVAL 10 //TODO A! change back to 10 /*!< Interval in seconds between generating CSNP message.*/
#define ISIS_PSNP_INTERVAL 2 /*!< Interval in seconds between generating PSNP message.*/
#define ISIS_LSP_MAX_SIZE  1492 /*!< Maximum size of LSP in Bytes.*/ //TODO change to something smaller so we can test it
#define ISIS_LSP_SEND_INTERVAL 5 /*!< Interval in seconds between periodic scanning LSP Database and checking SRM and SSN flags.*/
#define ISIS_SPF_FULL_INTERVAL 50 //TODO A! change back to 50
#define ISIS_TRILL_MAX_HELLO_SIZE 1470
//class InterfaceEntr;
//class MacAddress;





/* Adjacency state types according to TRILL specs in RFC6327 3.2
 *
 */
typedef enum{
    ISIS_ADJ_DOWN, //doesn't exist
    ISIS_ADJ_DETECT, // FALSE
    ISIS_ADJ_2WAY , // semiFALSE (this state is skipped in L3_ISIS)
    ISIS_ADJ_REPORT // previously represented as TRUE
}ISISAdjState;

/**
 * Structure for storing info about neighbours
 */
struct ISISadj
{
//    unsigned char sysID[6];             /*!<system ID of neighbour*/
    SystemID sysID;

//    unsigned char areaID[3];            /*!<neighbour areaID*/
    AreaId areaID;

    MacAddress mac;                     /*!<mac address of neighbour*/
    ISISAdjState state;                         /*!<adjacency state has to be 2-way; 0 = only 1 way, 1 = 2-way (hello received from adj router)*/
    ISISTimer *timer;                   /*!<timer set to hold time and reseted every time hello from neighbour is received. For L2_MODE works as designated VLAN holding timer*/
    ISISTimer *nonDesTimer;              /*!< nonDesignated VLAN holding timer */
    int interfaceId;                    /*!< interface id, which is neighbour connected to*/
//    int gateIndex;                      /*!<index of gate, which is neighbour connected to*/
    bool network;                       /*!<network type, true = broadcast, false = point-to-point*/
//    ISISTimer *desVLANTimer;  //see above
//    ISISTimer *nonDesVLANTimer;
    int priority; //priority of the neighbour to be the DRB
    int desiredDesVLAN; // neighbour's desired VLAN to the designated VLAN

    //bool operator for sorting
    bool operator<(const ISISadj& adj2) const {

        if(areaID == adj2.areaID){
            if(sysID == adj2.sysID){
                return mac.compareTo(adj2.mac) < 0;
            }else{
                return sysID < adj2.sysID;
            }
        }else{
            return areaID < adj2.areaID;
        }
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
//    unsigned char LANid[7]; /*!LAN ID = System ID (6B)+ Pseudonode ID (1B)*/
    PseudonodeID LANid;
    metrics_t metrics;      /*!metric*/

};


//TODO Delete unused
//struct LSPrecord
//{
//    std::vector<LSPneighbour> neighbours;    //list of neighbours
////    unsigned char LSPid[8];             //ID of LSP
//    LspID LSPid;
//    unsigned long seq;                  //sequence number
//    ISISTimer *deadTimer;               //dead timer - 1200s
//};



struct LSPRecord
{
public:
    Ptr<ISISLSPPacket> LSP; //link-state protocol data unit
    ISISTimer *deadTimer; //dead timer
private:
    std::vector<bool> srmFlags;
    std::vector<bool> ssnFlags;
    std::vector<int> interfaceIds;
public:
    double simLifetime; /*!< specify deadTi */

    //bool operator for sorting
    bool operator<(const LSPRecord& lspRec2) const
    {
      return LSP->getLspID() < lspRec2.LSP->getLspID();

    }

    int getFlagsSize(){
        return srmFlags.size();
    }

    bool getSrmFlag(int interfaceId){
        return srmFlags.at(getIndex(interfaceId));
    }

    void setSrmFlag(int interfaceId, bool value){
        srmFlags.at(getIndex(interfaceId)) = value;
    }

    void clearSrmFlag(int interfaceId){
        srmFlags.at(getIndex(interfaceId)) = false;
    }

    bool getSsnFlag(int interfaceId){
        return ssnFlags.at(getIndex(interfaceId));
    }

    void setSsnFlag(int interfaceId, bool value){
        ssnFlags.at(getIndex(interfaceId)) = value;
    }

    void clearSsnFlag(int interfaceId){
        ssnFlags.at(getIndex(interfaceId)) = false;
    }

    int getInterfaceId(int iterator){
        return interfaceIds.at(iterator);
    }

    void addFlags(bool srmFlag, bool ssnFlag, int interfaceId){
        srmFlags.push_back(false);
        ssnFlags.push_back(false);
        interfaceIds.push_back(interfaceId);

    }

    ~LSPRecord()
    {

      this->LSP->setTLVArraySize(0);
      if (this->LSP != NULL)
      {
          //TODO ANSAINET4.0 Uncomment delete?
//        delete this->LSP;
      }

      this->srmFlags.clear();
      this->ssnFlags.clear();
      this->interfaceIds.clear();
    }

private:
    int getIndex(int interfaceId){
        for(int i = 0; i < interfaceIds.size(); i++){
            if(interfaceIds.at(i) == interfaceId){
                return i;
            }
        }

        return -1;
    }

};
typedef std::vector<LSPRecord *> LSPRecQ_t;


struct cmpLSPRecord
{
    bool operator()(const LSPRecord *lspRec1, const LSPRecord *lspRec2)
    {
      return lspRec1->LSP->getLspID() < lspRec2->LSP->getLspID();

    }
};

struct FlagRecord
{
        LSPRecord *lspRec;
        int interfaceId;
        //destructor!!
        ~FlagRecord(){

        }
};
typedef std::vector<FlagRecord*> FlagRecQ_t;
typedef std::vector<std::vector<FlagRecord*> *> FlagRecQQ_t;

struct ISISNeighbour
{
//        unsigned char *id;
        PseudonodeID id;
        //uint32_t metric;
        bool type; //should represent whether it's a leaf node; true = leaf
        InterfaceEntry *entry;

        ISISNeighbour(){

        }

        ISISNeighbour(PseudonodeID pseudoId, bool type, InterfaceEntry *entry){
//            this->id = new unsigned char [ISIS_SYSTEM_ID + 2];
//            memcpy(this->id, id, ISIS_SYSTEM_ID + 2);
            id = pseudoId;
            this->type = false;
            this->entry = entry;
        }

        ~ISISNeighbour(){
//            delete id;
        }

        ISISNeighbour *copy(){
            return new ISISNeighbour(PseudonodeID(id), this->type, this->entry);
        }

};

typedef std::vector<ISISNeighbour*> ISISNeighbours_t;

struct ISISPath
{
    //        unsigned char *to;
    PseudonodeID to;
    uint32_t metric;
    ISISNeighbours_t from; // works as next hop
    //bool operator for sorting

    bool operator()(const ISISPath *path1, const ISISPath *path2)
    {

      if (path1->metric != path2->metric)
      {
        return path1->metric < path2->metric; //first is smaller, so return true
      }
      else
      {
        return path1->to < path2->to;

      }

    }

    ISISPath()
    {

    }

    ISISPath(PseudonodeID to, uint32_t metric, ISISNeighbours_t from)
    {
      this->to = to;
      this->metric = metric;
      for (ISISNeighbours_t::iterator it = from.begin(); it != from.end(); ++it)
      {
        this->from.push_back((*it)->copy());
      }

    }

    ISISPath* copy()
    {
      return new ISISPath(PseudonodeID(to), this->metric, this->from);
    }

};


typedef std::vector<ISISPath*> ISISPaths_t;

struct ISISAPath
{
    //        unsigned char *to;
    AreaId to;
    uint32_t metric;
    ISISNeighbours_t from; // works as next hop
    //bool operator for sorting

    bool operator()(const ISISAPath *path1, const ISISAPath *path2)
    {

      if (path1->metric != path2->metric)
      {
        return path1->metric < path2->metric; //first is smaller, so return true
      }
      else
      {
        return path1->to < path2->to;

      }

    }

    ISISAPath()
    {

    }

    ISISAPath(AreaId to, uint32_t metric, ISISNeighbours_t from)
    {
      this->to = to;
      this->metric = metric;
      for (ISISNeighbours_t::iterator it = from.begin(); it != from.end(); ++it)
      {
        this->from.push_back((*it)->copy());
      }

    }

    ISISAPath* copy()
    {
      return new ISISAPath(AreaId(to), this->metric, this->from);
    }

};


typedef std::vector<ISISAPath*> ISISAPaths_t;




typedef std::vector<LSPRecord *> ISISLspDb_t;

struct ISISCon
{

        PseudonodeID from; //or maybe just SystemID?
        PseudonodeID to; //or maybe just SystemID?
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

typedef enum
{
    ISIS_NETWORK_P2P,
    ISIS_NETWORK_BROADCAST

} ISISNetworkType;

typedef enum
{
    ISIS_CIRCUIT_RESERVED,
    ISIS_CIRCUIT_L1,
    ISIS_CIRCUIT_L2,
    ISIS_CIRCUIT_L1L2
} ISISCircuitType;

/**
 * Structure for storing info about all active interfaces.
 */
struct ISISinterface
{
    int interfaceId;                /*!<interface ID*/
//    int gateIndex;            /*!<gate index*/
    bool network;             /*!< network type, true = broadcast, false = point-to-point */
    //bool broadcast;         /*!<broadcast enabled?*/
    //bool loopback;          /*!<is it loopback?*/
    bool passive;             /*!<is it passive intf?*/
    bool ISISenabled;         /*!<is IS-IS activated on this interface? (default yes for all ifts)*/
    short circuitType;        /*!<circuit type  L1, L2, L1L2*/
    uint priority;   /*!<interface priority for being designated IS*/
    uint L1DISpriority;    /*!<priority of current L1 DIS*/
    uint L2DISpriority;    /*!<priority of currend L2 DIS*/
//    unsigned char L1DIS[ISIS_SYSTEM_ID + 1];   /*!<L1 designated router ID for ift*/
//    unsigned char L2DIS[ISIS_SYSTEM_ID + 1];   /*!<L2 designated router ID for ift*/
    PseudonodeID L1DIS;
    PseudonodeID L2DIS;
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
    FlagRecQ_t* l1srmBQueue;
    FlagRecQ_t* l1srmPTPQueue;
    FlagRecQ_t* l2srmBQueue;
    FlagRecQ_t* l2srmPTPQueue;
    FlagRecQ_t* l1ssnBQueue;
    FlagRecQ_t* l1ssnPTPQueue;
    FlagRecQ_t* l2ssnBQueue;
    FlagRecQ_t* l2ssnPTPQueue;

    InterfaceEntry *entry;    /*!< other interface info*/
};
typedef std::vector<ISISinterface> ISISInterTab_t;



} //end namespace inet



#endif /* ISISTYPES_H_ */
