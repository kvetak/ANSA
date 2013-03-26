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
 * @file ISIS.h
 * @author Matej Hrncirik, Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 7.3.2012
 * @brief
 * @detail
 * @todo TODO Incrementing sequence number should be done by using modulo and overflow should be properly handled
 *       FIXED BUG-ID: 1; A known bug emerged with new version of INET. If gateIndex doesn't corresponds to interface's index in interface table, things go terribly wrong.
 */

#ifndef ISIS_H_
#define ISIS_H_

#include <algorithm>
#include <stdlib.h>
#include <omnetpp.h>
#include <string>
#include <vector>
#include <queue>
#include <iomanip>

#include "CLNSTable.h"
#include "CLNSTableAccess.h"
#include "ISISMessage_m.h"
//#include "ISISLSPPacket.h"
#include "Ieee802Ctrl_m.h"
#include "MACAddress.h"
//#include "AnsaInterfaceTable.h"
//#include "AnsaInterfaceTableAccess.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "InterfaceStateManager.h"
#include "RoutingTableAccess.h"
#include "IRoutingTable.h"
//#include "IPRoute.h"
#include "IPv4Route.h"
#include "ISISTimer_m.h"
#include "xmlParser.h"
#include "ISIStypes.h"
#include <cmessage.h>
#include <crng.h>
//#include "TRILL.h"
class TRILL;

//class ISISLSPPacket;
/**
 * Single class providing all functionality of whole module.
 */
class ISIS : public cSimpleModule, protected INotifiable
{
    public:
        enum ISIS_MODE
        {
            L2_ISIS_MODE = 1,
            L3_ISIS_MODE = 2};
private:
    IInterfaceTable *ift; /*!< pointer to interface table */
    std::vector<ISISinterface> ISISIft; /*!< vector of available interfaces */
    CLNSTable *clnsTable; /*!< pointer to CLNS routing table */
    NotificationBoard *nb; /*!< Provides access to the notification board */
    TRILL *trill; /*!< Pointer to TRILL module, NULL if mode is L3_ISIS_MODE */
    ISIS_MODE mode;

    string deviceType; /*!< device type specified in .ned when using this module */
    string deviceId; /*!< device ID */
    string configFile; /*!< config file specified in simulation */
    string netAddr; /*!<  OSI network address in simplified NSAP format */
    unsigned char *areaId; /*!< first 3Bytes of netAddr as area ID */
    unsigned char *sysId; /*!< next 6Bytes of NetAddr as system ID */
    unsigned char *NSEL; /*!< last 1Byte of Netaddr as NSEL identifier */
    AdjTab_t adjL1Table; /*!< table of L1 adjacencies */
    AdjTab_t adjL2Table; /*!< table of L2 adjacencies */
    short isType; /*!< defines router IS-IS operational mode (L1,L2,L1L2) */
    std::vector<LSPrecord> L1LSP; /*!< L1 LSP database */
    std::vector<LSPrecord> L2LSP; /*!< L2 LSP database */
    LSPRecQ_t *L1LSPDb; /*!< L1 LSP database */
    LSPRecQ_t *L2LSPDb; /*!< L2 LSP database */
    FlagRecQQ_t *L1SRMBQueue; /*!< Vector with vector for each interface containing LSPRecords with SRMflag set */
    FlagRecQQ_t *L1SRMPTPQueue;
    FlagRecQQ_t *L2SRMBQueue; /*!< Vector with vector for each interface containing LSPRecords with SRMflag set */
    FlagRecQQ_t *L2SRMPTPQueue;
    FlagRecQQ_t *L1SSNBQueue; /*!< Vector with vector for each interface containing LSPRecords with SSNflag set */
    FlagRecQQ_t *L1SSNPTPQueue;
    FlagRecQQ_t *L2SSNBQueue; /*!< Vector with vector for each interface containing LSPRecords with SSNflag set */
    FlagRecQQ_t *L2SSNPTPQueue;
    ISISPaths_t *L1ISISPathsISO; /*!< Vector of best paths for ISO addressing. */
    ISISPaths_t *L2ISISPathsISO; /*!< Vector of best paths for ISO addressing. */
    //ISISPaths_t ISISTent;
    //ISISCons_t ISISInit;
    bool att; /*!< Attached flag. When set to true it indicates that this IS is attached to another Area and therefore could be "default gateway" for L1 ISs. This flag can be set to true only for L1L2 ISs.*/
    ISISNeighbours_t *attIS; /*!< System-ID of the closest L1L2 attached IS */
    int L1HelloInterval; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    int L2HelloInterval; /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    short L1HelloMultiplier; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
    short L2HelloMultiplier; /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */
    int lspInterval; /*!< Minimum delay in ms between sending two successive LSPs.*/
    int lspRefreshInterval; /*!< Interval in seconds in at which LSPs are refreshed (1 to 65535). This value SHOULD be less than ISIS_LSP_MAX_LIFETIME*/
    int lspMaxLifetime; /*!< Interval in seconds during which is specified LSP valid. This value SHOULD be more than ISIS_LSP_REFRESH_INTERVAL */
    int L1LspGenInterval; /*!< Interval in seconds at which LSPs are generated.*/
    int L2LspGenInterval; /*!< Interval in seconds at which LSPs are generated.*/
    int L1LspSendInterval; /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
    int L2LspSendInterval; /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
    int L1LspInitWait; /*!< Initial wait interval in ms before generating first LSP.*/
    int L2LspInitWait; /*!< Initial wait interval in ms before generating first LSP.*/
    int L1CSNPInterval; /*!< Interval in seconds between generating CSNP message.*/
    int L2CSNPInterval; /*!< Interval in seconds between generating CSNP message.*/
    int L1PSNPInterval; /*!< Interval in seconds between generating PSNP message.*/
    int L2PSNPInterval; /*!< Interval in seconds between generating PSNP message.*/
    int L1SPFFullInterval;
    int L2SPFFullInterval;
    unsigned long helloCounter; /*!< my hax hello counter to sync DIS/non-DIS hellos. This variable is deprecated, but is kept for sentimental reasons. */

    /* Init */
    void initISIS(); // main init
    void initHello();
    void initTRILLHello();
    void initGenerate();
    void initRefresh();
    void initPeriodicSend();
    void initCsnp();
    void initPsnp();
    void initSPF();

    /* Hello */
    void handlePTPHelloMsg(ISISMessage *inMsg); //
    void sendHelloMsg(ISISTimer *timer); // send hello messages
    void sendBroadcastHelloMsg(int interfaceIndex, int gateIndex, short  circuitType);
    void sendPTPHelloMsg(int interfaceIndex, int gateIndex, short  circuitType);
    void sendTRILLHelloMsg(ISISTimer *timer); //mimic the sendHelloMsg functionality
    void sendTRILLBroadcastHelloMsg(int interfaceIndex, int gateIndex, short circuitType);
    void sendTRILLPTPHelloMsg(int interfaceIndex, int gateIndex, short circuitType);
    unsigned short getHoldTime(int interfaceIndex, short  circuitType = L1_TYPE);
    double getHelloInterval(int interfaceIndex, short  circuitType); //return hello interval for specified interface and circuitType. For DIS interface returns only 1/3 of actual value;
    ISISadj *getAdjByGateIndex(int gateIndex, short  circuitType, int offset = 0); // return something corresponding to adjacency on specified link
    ISISadj *getAdjBySystemID(unsigned char *systemID, short  circuitType, int gateIndex = -1);
    ISISadj *getAdj(ISISMessage *inMsg, short  circuitType = L1_TYPE); //returns adjacency representing sender of inMsg or NULL when ANY parameter of System-ID, MAC address and gate index doesn't match
    ISISinterface *getIfaceByGateIndex(int gateIndex); //return ISISinterface for specified gateIndex
    bool isAdjBySystemID(unsigned char *systemID, short  circuitType); //do we have adjacency for systemID on specified circuitType
    bool isUp(int gateIndex, short  circuitType); //returns true if ISISInterface specified by the corresponding gateIndex have at least one adjacency in state UP
    bool amIL1DIS(int interfaceIndex); //returns true if specified interface is DIS
    bool amIL2DIS(int interfaceIndex); //returns true if specified interface is DIS
    bool amIDIS(int interfaceIndex, short  circuitType);
    void electDIS(ISISLANHelloPacket *msg);
    std::vector<ISISadj> *getAdjTab(short  circuitType);

    /* LSP */
    unsigned char *getLanID(ISISLANHelloPacket *msg);
    void handleLsp(ISISLSPPacket *lsp);
    void handleCsnp(ISISCSNPPacket *csnp);
    void handlePsnp(ISISPSNPPacket *psnp);
    std::vector<unsigned char*> *getLspRange(unsigned char *startLspID, unsigned char *endLspID, short  circuitType);
    unsigned char *getStartLspID(ISISCSNPPacket *csnp);
    unsigned char *getEndLspID(ISISCSNPPacket *csnp);
    unsigned char *getLspID(ISISLSPPacket *msg);
    void setLspID(ISISLSPPacket *msg, unsigned char *lspID);
    void updateLSP(ISISLSPPacket *lsp, short  circuitType);
    void replaceLSP(ISISLSPPacket *lsp, LSPRecord *lspRecord, short  circuitType);
    void purgeRemainLSP(unsigned char *lspId, short  circuitType);
    void purgeLSP(unsigned char *lspId, short  circuitType); //purge in-memory LSP
    void purgeLSP(ISISLSPPacket *lsp, short  circuitType); //purge incomming LSP
    void purgeMyLSPs(short  circuitType);
    void deleteLSP(ISISTimer *timer); //delete (already purged)LSP from DB when appropriate timer expires
    void sendLSP(LSPRecord *lspRec, int gateIndex);
    std::vector<ISISLSPPacket*> *genLSP(short  circuitType);
    void generateLSP(ISISTimer *timer);
    void generateLSP(short  circuitType);
    void refreshLSP(ISISTimer *timer);
    void refreshLSP(short  circuitType);
    std::vector<LSPRecord*> *getLSPDb(short  circuitType);
    void periodicSend(ISISTimer *timer, short  circuitType); /*!< Sends LSPs on interfaces with SRMflag every ISIS_LSP_GEN_INTERVAL.*/
    void sendCsnp(ISISTimer *timer);
    void sendPsnp(ISISTimer *timer);
    unsigned char *getLSPID();
    //double getHoldtimeInterval(int interfaceIndex, short circuitType); //return hold time interval for specified interface and circuitType.
    LSPRecord *getLSPFromDbByID(unsigned char *LSPID, short  circuitType);
    bool compareLSP(ISISLSPPacket *lsp1, ISISLSPPacket *lsp2);
    LSPRecord *installLSP(ISISLSPPacket *lsp, short  circuitType); //install lsp into local LSP database
    void updateAtt(bool action); /*!< Action specify whether this method has been called to set (true - new adjacency) or clear (false - removing adjacency) Attached flag. */
    void setClosestAtt(void); /*!< Find and set the closest L1L2 attached IS */

    /* SPF */
    void fullSPF(ISISTimer *timer);
    bool extractISO(ISISCons_t *initial, short  circuitType); /*!< Extracts ISO informations from lspDb needed to perform SPF calculation. > */
    ISISPath *getPath(ISISPaths_t *paths, unsigned char *id);
    ISISCons_t *getCons(ISISCons_t *cons, unsigned char *from);
    void getBestMetric(ISISPaths_t *paths);
    ISISPath *getBestPath(ISISPaths_t *paths);
    void twoWayCheck(ISISCons_t *cons);
    bool isCon(ISISCons_t *cons, unsigned char *from, unsigned char *to);
    void bestToPath(ISISCons_t *cons, ISISPaths_t *ISISTent, ISISPaths_t *ISISPaths);
    void moveToTent(ISISCons_t *initial, ISISPath *path, unsigned char *from, uint32_t metric, ISISPaths_t *ISISTent);
    void moveToPath(ISISPath *path);
    void extractAreas(ISISPaths_t *paths, ISISPaths_t *areas, short  circuitType);
    ISISPaths_t *getPathsISO(short  circuitType);

    /* Flags */
    FlagRecQQ_t *getSRMPTPQueue(short  circuitType);
    FlagRecQQ_t *getSRMBQueue(short  circuitType);
    FlagRecQQ_t *getSSNPTPQueue(short  circuitType);
    FlagRecQQ_t *getSSNBQueue(short  circuitType);
    FlagRecQ_t *getSRMQ(bool network, int interfaceIndex, short  circuitType);
    FlagRecQ_t *getSSNQ(bool network, int interfaceIndex, short  circuitType);
    void setSRMflag(LSPRecord *lspRec, int interfaceIndex, short  circuitType);
    void setSRMflags(LSPRecord *lspRec, short  circuitType);
    void setSRMflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short  circuitType);
    void clearSRMflag(LSPRecord *lspRec, int interfaceIndex, short  circuitType);
    void clearSRMflags(LSPRecord *lspRec, short  circuitType);
    void clearSRMflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short  circuitType);
    void setSSNflag(LSPRecord *lspRec, int interfaceIndex, short  circuitType);
    void setSSNflags(LSPRecord *lspRec, short  circuitType);
    void setSSNflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short  circuitType);
    void clearSSNflag(LSPRecord *lspRec, int interfaceIndex, short  circuitType);
    void clearSSNflags(LSPRecord *lspRec, short  circuitType);
    void clearSSNflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short  circuitType);
    void addFlags(LSPRecord *lspRec, short  circuitType); //add and set SRM and SSN flags for lspRec

    /* Print */
    void printLSP(ISISLSPPacket *lsp, char *from);
    void printSysId(unsigned char *sysId);
    void printLspId(unsigned char *lspId);
    void printPaths(ISISPaths_t *paths);
    void schedule(ISISTimer *timer, double timee = -1); //if timer needs additional information there is msg
    unsigned char *getSysID(ISISMessage *msg);
    unsigned char *getSysID(ISISTimer *timer);
    unsigned char *getLspID(ISISTimer *timer);
    /*    unsigned char* getSysID(ISISL1HelloPacket *msg);
         unsigned char* getSysID(ISISL2HelloPacket *msg);
         unsigned char* getSysID(ISISPTPHelloPacket *msg);*/

    /* TLV */
    void addTLV(ISISMessage *inMsg, enum TLVtypes tlvType, short  circuitType, int gateIndex = -1); //generate and add this tlvType LSP to message
    void addTLV(ISISMessage *inMsg, TLV_t *tlv); //add this TLV in message
    void addTLV(std::vector<TLV_t*> *tlvtable, enum TLVtypes tlvType, short  circuitType, unsigned char nsel = 0);
    TLV_t *genTLV(enum TLVtypes tlvType, short  circuitType, int gateIndex = -1);
    bool isAdjUp(short  circuitType); //returns true if ANY adjacency is up on specified circuit level
    bool isAdjUp(ISISMessage *msg, short  circuitType);
    TLV_t *getTLVByType(ISISMessage *inMsg, enum TLVtypes tlvType, int offset = 0);
    /*    TLV_t* getTLVByType(ISISL1HelloPacket *msg,enum TLVtypes tlvType, int offset = 0);
         TLV_t* getTLVByType(ISISL2HelloPacket *msg,enum TLVtypes tlvType, int offset = 0);
         TLV_t* getTLVByType(ISISPTPHelloPacket *msg,enum TLVtypes tlvType, int offset = 0);*/
    bool isMessageOK(ISISMessage *inMsg);
    bool isAreaIDOK(TLV_t *areaAddressTLV, unsigned char *compare = NULL); //if compare is NULL then use this->areaId for comparison
    int getIfaceIndex(ISISinterface *interface); //returns index to ISISIft

    /* General */
    short getLevel(ISISMessage *msg); //returns level (circuitType) of the message


    bool parseNetAddr(); // validate and parse net address format
    void handleL1HelloMsg(ISISMessage *inMsg); // handle L1 hello messages
    void handleL2HelloMsg(ISISMessage *inMsg); // handle L2 hello messages
    void removeDeadNeighbour(ISISTimer *msg); //remove dead neighbour when timer message arrives
    void electL1DesignatedIS(ISISL1HelloPacket *msg); //starts election of L1 designated intermediate system
    void electL2DesignatedIS(ISISL2HelloPacket *msg); //starts election of L2 designated intermediate system
    void resetDIS(unsigned char *systemID, int interfaceIndex, short  circuitType); //resetDIS only if system specified in timer was DIS on interface specified in timer                 //resets DIS on al interfaces for L1/L2
    void sendMyL1LSPs(); //send content of my L1 link-state DB
    void sendMyL2LSPs(); //send content of my L2 link-state DB
    void floodFurtherL1LSP(ISISLSPL1Packet *msg); //send recived LSP packet further to all other neighbours
    void sendSpecificL1LSP(unsigned char *LSPid); //send only specific LSP
    void handleL1LSP(ISISMessage *msg); //handle L1 LSP packets
    void handleL2LSP(ISISMessage *msg); //handle L2 LSP packets
    void sendL1CSNP(); //send L1 CSNP packets
    void sendL2CSNP(); //send L2 CSNP packets
    void handleL1CSNP(ISISMessage *msg); //handle L1 CSNP packets
    void handleL2CSNP(ISISMessage *msg); //handle L2 CSNP packets
    void sendL1PSNP(std::vector<unsigned char*> *LSPlist, int gateIndex); //send L1 PSNP packets
    void sendL2PSNP(); //send L2 PSNP packets
    void handleL1PSNP(ISISMessage *msg); //handle L1 PSNP packets
    void handleL2PSNP(ISISMessage *msg); //handle L2 PSNP packets
    bool checkDuplicateSysID(ISISMessage *msg); //check sysId field from received hello packet for duplicated sysId
    void removeDeadLSP(ISISTimer *msg); //remove expired LSP
    void updateMyLSP(); //create or update my own LSPs

protected:
    virtual void initialize(int stage); //init
    virtual void handleMessage(cMessage *msg); //basic message handling
    virtual int numInitStages() const
    {
        return 5;
    }

    bool compareArrays(unsigned char *first, unsigned char *second, unsigned int size); //method for comparison of two unsigned int arrays
    void copyArrayContent(unsigned char *src, unsigned char *dst, unsigned int size, unsigned int startSrc, unsigned int startDst); //copy content from one array to another
    virtual void receiveChangeNotification(int category, const cObject *details);

public:
    ISIS();
    virtual ~ISIS(); //destructor
    void insertIft(InterfaceEntry *entry, cXMLElement *device); //insert new interface to vector
    void appendISISInterface(ISISinterface iface);
    void printAdjTable(); //print adjacency table
    void printLSPDB(); //print content of link-state database
    void setClnsTable(CLNSTable *clnsTable);
    void setTrill(TRILL *trill);
    void setIft(IInterfaceTable *ift);
    void setNb(NotificationBoard *nb);
    void subscribeNb(void);
    void setMode(ISIS_MODE mode);
    void setNetAddr(string netAddr);

    void generateNetAddr();
    void setIsType(short  isType);
    void setHelloCounter(unsigned long  helloCounter);
    void setL1CSNPInterval(int l1CSNPInterval);
    void setL1HelloInterval(int l1HelloInterval);
    void setL1HelloMultiplier(short  l1HelloMultiplier);
    void setL1LspGenInterval(int l1LspGenInterval);
    void setL1LspInitWait(int l1LspInitWait);
    void setL1LspSendInterval(int l1LspSendInterval);
    void setL1PSNPInterval(int l1PsnpInterval);
    void setL1SpfFullInterval(int l1SpfFullInterval);
    void setL2CSNPInterval(int l2CsnpInterval);
    void setL2HelloInterval(int l2HelloInterval);
    void setL2HelloMultiplier(short  l2HelloMultiplier);
    void setL2LspGenInterval(int l2LspGenInterval);
    void setL2LspInitWait(int l2LspInitWait);
    void setL2LspSendInterval(int l2LspSendInterval);
    void setL2PSNPInterval(int l2PsnpInterval);
    void setL2SpfFullInterval(int l2SpfFullInterval);
    void setLspInterval(int lspInterval);
    void setLspMaxLifetime(int lspMaxLifetime);
    void setLspRefreshInterval(int lspRefreshInterval);
    const unsigned char *getSysId() const;
    short getIsType() const;
    int getL1HelloInterval() const;
    short getL1HelloMultiplier() const;
    int getL2HelloInterval() const;
    short getL2HelloMultiplier() const;
    int getLspInterval() const;
    int getL1CsnpInterval() const;
    int getL1PsnpInterval() const;
    int getL2CsnpInterval() const;
    int getL2PsnpInterval() const;
    void setL1CsnpInterval(int l1CsnpInterval);
    void setL1PsnpInterval(int l1PsnpInterval);
    void setL2CsnpInterval(int l2CsnpInterval);
    void setL2PsnpInterval(int l2PsnpInterval);
    ISIS_MODE getMode() const;
    int getISISIftSize();
};


#endif /* ISIS_H_ */
