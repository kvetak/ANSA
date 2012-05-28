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

#include <omnetpp.h>
#include <string>
#include <vector>
#include <iomanip>
#include "ISISMessage_m.h"
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

/**
 * Single class providing all functionality of whole module.
 */
class ISIS : public cSimpleModule
{
private:

    IInterfaceTable *ift;                       /*!< pointer to interface table */
    std::vector<ISISinterface> ISISIft;         /*!< vector of available interfaces */
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
    unsigned long helloCounter;                 /*!< my hax hello counter to sync DIS/non-DIS hellos */

    void insertIft(InterfaceEntry *entry, cXMLElement *device);          //insert new interface to vector
    void sendHelloMsg();                         // send hello messages
    bool parseNetAddr();                         // validate and parse net address format
    void handleL1HelloMsg(ISISMessage *inMsg);   // handle L1 hello messages
    void handleL2HelloMsg(ISISMessage *inMsg);   // handle L2 hello messages
    void removeDeadNeighbour(ISISTimer *msg);    //remove dead neighbour when timer message arrives
    void electL1DesignatedIS(ISISL1HelloPacket *msg);                 //starts election of L1 designated intermediate system
    void electL2DesignatedIS(ISISL2HelloPacket *msg);                  //starts election of L2 designated intermediate system
    void resetDIS(short IStype);                 //resets DIS on al interfaces for L1/L2
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
    virtual int numInitStages() const  {return 4;}
    bool compareArrays( unsigned char * first, unsigned char * second, unsigned int size);          //method for comparison of two unsigned int arrays
    void copyArrayContent(unsigned char * src, unsigned char * dst, unsigned int size, unsigned int startSrc, unsigned int startDst);   //copy content from one array to another

public:
    virtual ~ISIS();                            //destructor
    void printAdjTable();                       //print adjacency table
    void printLSPDB();                          //print content of link-state database
};


#endif /* ISIS_H_ */
