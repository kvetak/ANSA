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

#ifndef ISIS_H_
#define ISIS_H_

#include <algorithm>
#include <stdlib.h>
#include <omnetpp.h>
#include <string>
#include <vector>
#include <queue>
#include <iomanip>

#include "ISISMessage_m.h"
//#include "ISISLSPPacket.h"
#include "Ieee802Ctrl_m.h"
#include "MACAddress.h"
#include "AnsaInterfaceTable.h"
#include "AnsaInterfaceTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "InterfaceStateManager.h"
#include "RoutingTableAccess.h"
#include "IRoutingTable.h"
#include "IPRoute.h"
#include "ISISTimer_m.h"
#include "xmlParser.h"
#include "ISIStypes.h"
#include <cmessage.h>
#include <crng.h>

//class ISISLSPPacket;
/**
 * Single class providing all functionality of whole module.
 */
class ISIS : public cSimpleModule
{
    private:

        IInterfaceTable *ift; /*!< pointer to interface table */
        std::vector<ISISinterface> ISISIft; /*!< vector of available interfaces */
    const char *deviceType;                     /*!< device type specified in .ned when using this module */
    const char *deviceId;                       /*!< device ID */
    const char *configFile;                     /*!< config file specified in simulation */
    const char *netAddr;                        /*!<  OSI network address in simplified NSAP format */
    const unsigned char *areaId;                /*!< first 3Bytes of netAddr as area ID */
    const unsigned char *sysId;                 /*!< next 6Bytes of NetAddr as system ID */
    const unsigned char *NSEL;                  /*!< last 1Byte of Netaddr as NSEL identifier */
    std::vector<ISISadj> adjL1Table;            /*!< table of L1 adjacencies */
    std::vector<ISISadj> adjL2Table;            /*!< table of L2 adjacencies */
    short isType;                               /*!< defines router IS-IS operational mode (L1,L2,L1L2) */
    std::vector<LSPrecord> L1LSP;               /*!< L1 LSP database */
    std::vector<LSPrecord> L2LSP;               /*!< L2 LSP database */
    std::vector<LSPRecord *> *L1LSPDb;       /*!< L1 LSP database */
    std::vector<LSPRecord *> *L2LSPDb; /*!< L2 LSP database */
        std::vector<std::vector<FlagRecord*> *> *L1SRMBQueue; /*!< Vector with vector for each interface containing LSPRecords with SRMflag set */
        std::vector<std::vector<FlagRecord*> *> *L1SRMPTPQueue;
        std::vector<std::vector<FlagRecord*> *> *L2SRMBQueue; /*!< Vector with vector for each interface containing LSPRecords with SRMflag set */
        std::vector<std::vector<FlagRecord*> *> *L2SRMPTPQueue;

        std::vector<std::vector<FlagRecord*> *> *L1SSNBQueue; /*!< Vector with vector for each interface containing LSPRecords with SSNflag set */
        std::vector<std::vector<FlagRecord*> *> *L1SSNPTPQueue;
        std::vector<std::vector<FlagRecord*> *> *L2SSNBQueue; /*!< Vector with vector for each interface containing LSPRecords with SSNflag set */
        std::vector<std::vector<FlagRecord*> *> *L2SSNPTPQueue;


    int L1HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L1HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    int L2HelloInterval;                        /*!< Hello interval for Level 1, 1 - 65535, 0 value causes the system to compute the hello interval based on the hello multiplier (specified by the L2HelloMultiplier ) so that the resulting hold time is 1 second. On designated intermediate system (DIS) interfaces, only one third of the configured value is used. Default is 10. */
    short L1HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L1HelloMultiplier times the L1HelloInterval. Default is 3. */
    short L2HelloMultiplier;                    /*!< Value between 3 - 1000. The advertised hold time in IS-IS hello packets will be set to the L2HelloMultiplier times the L2HelloInterval. Default is 3. */
    int lspInterval;                            /*!< Minimum delay in ms between sending two successive LSPs.*/
    int lspRefreshInterval;                     /*!< Interval in seconds in at which LSPs are refreshed (1 to 65535). This value SHOULD be less than ISIS_LSP_MAX_LIFETIME*/
    int lspMaxLifetime;                         /*!< Interval in seconds during which is specified LSP valid. This value SHOULD be more than ISIS_LSP_REFRESH_INTERVAL */
    int L1LspGenInterval;                       /*!< Interval in seconds at which LSPs are generated.*/
    int L2LspGenInterval;                       /*!< Interval in seconds at which LSPs are generated.*/
    int L1LspSendInterval;                       /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
    int L2LspSendInterval;                       /*!< Interval in seconds at which LSPs (with SRMflag set) are transmitted.*/
    int L1LspInitWait;                            /*!< Initial wait interval in ms before generating first LSP.*/
    int L2LspInitWait;                            /*!< Initial wait interval in ms before generating first LSP.*/
    int L1CsnpInterval;                           /*!< Interval in seconds between generating CSNP message.*/
    int L2CsnpInterval;                           /*!< Interval in seconds between generating CSNP message.*/
    int L1PsnpInterval;                           /*!< Interval in seconds between generating PSNP message.*/
    int L2PsnpInterval;                           /*!< Interval in seconds between generating PSNP message.*/
    unsigned long helloCounter;                 /*!< my hax hello counter to sync DIS/non-DIS hellos. This variable is deprecated, but is kept for sentimental reasons. */


    void initISIS();       // main init
    void initHello();
    void initGenerate();
    void initRefresh();
    void initPeriodicSend();
    void initCsnp();
    void initPsnp();
    ISISadj* getAdjByGateIndex(int gateIndex, short circuitType, int offset = 0);  // return something corresponding to adjacency on specified link
    ISISadj* getAdjBySystemID(unsigned char *systemID, short circuitType, int gateIndex = -1);
    ISISinterface* getIfaceByGateIndex(int gateIndex); //return ISISinterface for specified gateIndex
    unsigned short getHoldTime(int interfaceIndex, short circuitType = L1_TYPE);
    double getHelloInterval(int interfaceIndex, short circuitType); //return hello interval for specified interface and circuitType. For DIS interface returns only 1/3 of actual value;
    //double getHoldtimeInterval(int interfaceIndex, short circuitType); //return hold time interval for specified interface and circuitType.
    bool amIL1DIS(int interfaceIndex);  //returns true if specified interface is DIS
    bool amIL2DIS(int interfaceIndex);  //returns true if specified interface is DIS
    void sendBroadcastHelloMsg(int interfaceIndex, short circuitType);
    void sendPTPHelloMsg(int interfaceIndex, short circuitType);
    void schedule(ISISTimer* timer, double timee = -1);//if timer needs additional information there is msg
    void handlePTPHelloMsg(ISISMessage *inMsg);  //
    bool isAdjBySystemID(unsigned char *systemID, short circuitType); //do we have adjacency for systemID on specified circuitType
    ISISadj* getAdj(ISISMessage *inMsg, short circuitType = L1_TYPE);//returns adjacency representing sender of inMsg or NULL when ANY parameter of System-ID, MAC address and gate index doesn't match
    unsigned char* getSysID(ISISMessage *msg);
    unsigned char* getSysID(ISISTimer *timer);
    unsigned char* getLspID(ISISTimer *timer);

/*    unsigned char* getSysID(ISISL1HelloPacket *msg);
    unsigned char* getSysID(ISISL2HelloPacket *msg);
    unsigned char* getSysID(ISISPTPHelloPacket *msg);*/

    unsigned char* getLspID(ISISLSPPacket *msg);
    void setLspID(ISISLSPPacket *msg, unsigned char * lspID);
    void updateLSP(ISISLSPPacket* lsp, short circuitType);

    void addTLV(ISISMessage *inMsg, enum TLVtypes tlvType);//generate and add this tlvType LSP to message
    void addTLV(ISISMessage *inMsg, TLV_t *tlv);//add this TLV in message
    void addTLV(std::vector<TLV_t *> *tlvtable, enum TLVtypes tlvType, short circuitType, unsigned char nsel = 0);

    bool isAdjUp(short circuitType); //returns true if ANY adjacency is up on specified circuit level
    bool isAdjUp(ISISMessage *msg, short circuitType);

    TLV_t* getTLVByType(ISISMessage *inMsg,enum TLVtypes tlvType, int offset = 0);
/*    TLV_t* getTLVByType(ISISL1HelloPacket *msg,enum TLVtypes tlvType, int offset = 0);
    TLV_t* getTLVByType(ISISL2HelloPacket *msg,enum TLVtypes tlvType, int offset = 0);
    TLV_t* getTLVByType(ISISPTPHelloPacket *msg,enum TLVtypes tlvType, int offset = 0);*/
    bool isMessageOK(ISISMessage *inMsg);
    bool isAreaIDOK(TLV_t* areaAddressTLV, unsigned char *compare = NULL);//if compare is NULL then use this->areaId for comparison
    int getIfaceIndex(ISISinterface *interface); //returns index to ISISIft

    LSPRecord* getLSPFromDbByID(unsigned char *LSPID, short circuitType);
    bool compareLSP(ISISLSPPacket *lsp1, ISISLSPPacket *lsp2);

    LSPRecord * installLSP(ISISLSPPacket *lsp, short circuitType);//install lsp into local LSP database
    void replaceLSP(ISISLSPPacket *lsp, LSPRecord *lspRecord, short circuitType);
    void purgeRemainLSP(unsigned char * lspId, short circuitType);
    void purgeLSP(unsigned char *lspId, short circuitType);//purge in-memory LSP
    void purgeLSP(ISISLSPPacket *lsp, short circuitType); //purge incomming LSP
    void purgeMyLSPs(short circuitType);

    void deleteLSP(ISISTimer *timer);//delete (already purged)LSP from DB when appropriate timer expires

    void sendLSP(LSPRecord *lspRec, int gateIndex);
    void printLSP(ISISLSPPacket *lsp, char *from);
    void printSysId(unsigned char *sysId);
    void printLspId(unsigned char *lspId);

    void setSRMflag(LSPRecord * lspRec, int index, short circuitType);
    void setSRMflags(LSPRecord *lspRec, short circuitType);
    void setSRMflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType);

        void clearSRMflag(LSPRecord *lspRec, int index, short circuitType);
        void clearSRMflags(LSPRecord *lspRec, short circuitType);
        void clearSRMflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType);

        void setSSNflag(LSPRecord * lspRec, int index, short circuitType);
        void setSSNflags(LSPRecord *lspRec, short circuitType);
        void setSSNflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType);

        void clearSSNflag(LSPRecord *lspRec, int index, short circuitType);
        void clearSSNflags(LSPRecord *lspRec, short circuitType);
        void clearSSNflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType);

        void addFlags(LSPRecord *lspRec, short circuitType); //add and set SRM and SSN flags for lspRec

        void periodicSend(ISISTimer *timer, short circuitType); /*!< Sends LSPs on interfaces with SRMflag every ISIS_LSP_GEN_INTERVAL.*/
   void sendL1Csnp(ISISTimer *timer);
   void sendL1Psnp(ISISTimer *timer);

    std::vector<ISISLSPPacket *>* genLSP(short circuitType);
    void generateLSP(ISISTimer *timer);
    void generateLSP(short circuitType);

    void refreshLSP(ISISTimer *timer);
    void refreshLSP(short circuitType);

    unsigned char * getLSPID();
    std::vector<LSPRecord *> * getLSPDb(short circuitType);
    std::vector<ISISadj> * getAdjTab(short circuitType);
    std::vector<std::vector<FlagRecord*>* > * getSRMPTPQueue(short circuitType);
    std::vector<std::vector<FlagRecord*>* > * getSRMBQueue(short circuitType);

    std::vector<std::vector<FlagRecord*>* > * getSSNPTPQueue(short circuitType);
    std::vector<std::vector<FlagRecord*>* > * getSSNBQueue(short circuitType);

    std::vector<FlagRecord*>* getSRMQ(bool network, int index, short circuitType);
    std::vector<FlagRecord*>* getSSNQ(bool network, int index, short circuitType);

    void handleL1Lsp(ISISLSPPacket *lsp);
    void handleL1Csnp(ISISCSNPPacket *csnp);
    void handleL1Psnp(ISISPSNPPacket *psnp);

    std::vector<unsigned char *>* getLspRange(unsigned char *startLspID, unsigned char * endLspID, short circuitType);
    unsigned char * getStartLspID(ISISCSNPPacket *csnp);
    unsigned char * getEndLspID(ISISCSNPPacket *csnp);

    bool isUp(int gateIndex, short circuitType);//returns true if ISISInterface specified by the corresponding gateIndex have at least one adjacency in state UP


    void insertIft(InterfaceEntry *entry, cXMLElement *device);          //insert new interface to vector
    void sendHelloMsg(ISISTimer* timer);                         // send hello messages
    bool parseNetAddr();                         // validate and parse net address format
    void handleL1HelloMsg(ISISMessage *inMsg);   // handle L1 hello messages
    void handleL2HelloMsg(ISISMessage *inMsg);   // handle L2 hello messages
    void removeDeadNeighbour(ISISTimer *msg);    //remove dead neighbour when timer message arrives
    void electL1DesignatedIS(ISISL1HelloPacket *msg);                 //starts election of L1 designated intermediate system
    void electL2DesignatedIS(ISISL2HelloPacket *msg);                  //starts election of L2 designated intermediate system
    void resetDIS(unsigned char* systemID, int gateIndex, short circuitType);    //resetDIS only if system specified in timer was DIS on interface specified in timer                 //resets DIS on al interfaces for L1/L2
    void sendMyL1LSPs();                         //send content of my L1 link-state DB
    void sendMyL2LSPs();                         //send content of my L2 link-state DB
    void floodFurtherL1LSP(ISISLSPL1Packet *msg);   //send recived LSP packet further to all other neighbours
    void sendSpecificL1LSP(unsigned char *LSPid);   //send only specific LSP
    void handleL1LSP(ISISMessage * msg);          //handle L1 LSP packets
    void handleL2LSP(ISISMessage * msg);          //handle L2 LSP packets
    void sendL1CSNP();                            //send L1 CSNP packets
    void sendL2CSNP();                            //send L2 CSNP packets
    void handleL1CSNP(ISISMessage * msg);         //handle L1 CSNP packets
    void handleL2CSNP(ISISMessage * msg);         //handle L2 CSNP packets
    void sendL1PSNP(std::vector<unsigned char *> * LSPlist, int gateIndex);  //send L1 PSNP packets
    void sendL2PSNP();                            //send L2 PSNP packets
    void handleL1PSNP(ISISMessage * msg);         //handle L1 PSNP packets
    void handleL2PSNP(ISISMessage * msg);         //handle L2 PSNP packets
    bool checkDuplicateSysID(ISISMessage * msg);  //check sysId field from received hello packet for duplicated sysId
    void removeDeadLSP(ISISTimer *msg);           //remove expired LSP
    void updateMyLSP();                           //create or update my own LSPs




protected:
    virtual void initialize(int stage);                   //init
    virtual void handleMessage(cMessage* msg);           //basic message handling
    virtual int numInitStages() const  {return 5;}
    bool compareArrays( unsigned char * first, unsigned char * second, unsigned int size);          //method for comparison of two unsigned int arrays
    void copyArrayContent(unsigned char * src, unsigned char * dst, unsigned int size, unsigned int startSrc, unsigned int startDst);   //copy content from one array to another

public:
    ISIS();
    virtual ~ISIS();                            //destructor
    void printAdjTable();                       //print adjacency table
    void printLSPDB();                          //print content of link-state database
};


#endif /* ISIS_H_ */
