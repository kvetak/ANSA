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
/**
 * @file AnsaIPv4Route.cc
 * @date 25.1.2013
 * @author Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz),
 * Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Inherited class from IPv4Route for PIM purposes
 * @detail File implements new functions for IPv4Route which are needed for PIM
 */

#ifndef ANSAIPV4ROUTE_H_
#define ANSAIPV4ROUTE_H_

#include "IPv4Route.h"

#include "PIMTimer_m.h"
#include "InterfaceEntry.h"


/**
 * Route flags. Added to each route.
 */
enum flag
{
    D,              /**< Dense */
    S,              /**< Sparse */
    C,              /**< Connected */
    P,              /**< Pruned */
    A,              /**< Source is directly connected */
    F,              /**< Register flag*/
    T               /**< SPT bit*/
};

/**
 * States of each outgoing interface. E.g.: Forward/Dense.
 */
enum intState
{
    Densemode = 1,
    Sparsemode = 2,
    Forward,
    Pruned
};

/**
 * Assert States of each outgoing interface.
 */
enum AssertState
{
    NoInfo = 0,
    Winner = 1,
    Loser = 2
};

/**
 * Register machine States.
 */
enum RegisterState
{
  NoInfoRS = 0,
  Join = 1,
  Prune = 2,
  JoinPending = 3
};

/**
 * @brief Structure of incoming interface.
 * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
 */
struct inInterface
{
    InterfaceEntry          *intPtr;            /**< Pointer to interface */
    int                     intId;              /**< Interface ID */
    IPv4Address               nextHop;            /**< RF neighbor */
};

/**
 * @brief Structure of outgoing interface.
 * @details E.g.: Ethernet0, Forward/Sparse, 5:29:15/0:02:57
 */
struct outInterface
{
    InterfaceEntry          *intPtr;            /**< Pointer to interface */
    int                     intId;              /**< Interface ID */
    intState                forwarding;         /**< Forward or Pruned */
    intState                mode;               /**< Dense, Sparse, ... */
    PIMpt                   *pruneTimer;        /**< Pointer to PIM Prune Timer*/
    AssertState             assert;             /**< Assert state. */
    RegisterState           regState;           /**< Register state. */
};

/**
 * Vector of outgoing interfaces.
 */
typedef std::vector<outInterface> InterfaceVector;




class INET_API AnsaIPv4MulticastRoute : public IPv4MulticastRoute
{
    private:
        IPv4Address                 RP;                     /**< Randevous point */
        std::vector<flag>           flags;                  /**< Route flags */
        // timers
        PIMgrt                      *grt;                   /**< Pointer to Graft Retry Timer*/
        PIMsat                      *sat;                   /**< Pointer to Source Active Timer*/
        PIMsrt                      *srt;                   /**< Pointer to State Refresh Timer*/
        PIMkat                      *kat;                   /**< Pointer to Keep Alive timer for PIM-SM*/
        PIMrst                      *rst;                   /**< Pointer to Register-stop timer for PIM-SM*/
        PIMet                       *et;                    /**< Pointer to Expiry timer for PIM-SM*/
        PIMjt                       *jt;                    /**< Pointer to Join timer*/
        // interfaces
        inInterface                 inInt;                  /**< Incoming interface */
        InterfaceVector             outInt;                 /**< Outgoing interface */

        bool                        showOutInt;             /**< indicate if it is necessary to show out interface in show ip mroute */


        //Originated from destination.Ensures loop freeness.
        unsigned int sequencenumber;
        //Time of routing table entry creation
        simtime_t installtime;

    public:
        AnsaIPv4MulticastRoute();                                                 /**< Set all pointers to null */
        virtual ~AnsaIPv4MulticastRoute() {}
        virtual std::string info() const;

    public:
        void setRP(IPv4Address RP)  {this->RP = RP;}                        /**< Set RP IP address */
        void setGrt (PIMgrt *grt)   {this->grt = grt;}                      /**< Set pointer to PimGraftTimer */
        void setSat (PIMsat *sat)   {this->sat = sat;}                      /**< Set pointer to PimSourceActiveTimer */
        void setSrt (PIMsrt *srt)   {this->srt = srt;}                      /**< Set pointer to PimStateRefreshTimer */
        void setKat (PIMkat *kat)   {this->kat = kat;}                      /**< Set pointer to KeepAliveTimer */
        void setRst (PIMrst *rst)   {this->rst = rst;}                      /**< Set pointer to RegisterStopTimer */
        void setEt  (PIMet *et)     {this->et = et;}                        /**< Set pointer to ExpiryTimer */
        void setJt  (PIMjt *jt)     {this->jt = jt;}                        /**< Set pointer to JoinTimer */

        void setFlags(std::vector<flag> flags)  {this->flags = flags;}      /**< Set vector of flags (flag) */
        bool isFlagSet(flag fl);                                            /**< Returns if flag is set to entry or not*/
        void addFlag(flag fl);                                              /**< Add flag to ineterface */
        void removeFlag(flag fl);                                           /**< Remove flag from ineterface */

        void setInInt(InterfaceEntry *interfacePtr, int intId, IPv4Address nextHop) {this->inInt.intPtr = interfacePtr; this->inInt.intId = intId; this->inInt.nextHop = nextHop;}    /**< Set information about incoming interface*/
        void setInInt(inInterface inInt) {this->inInt = inInt;}                                                         /**< Set incoming interface*/

        void setOutInt(InterfaceVector outInt) {EV << "MulticastIPRoute: New OutInt" << endl; this->outInt = outInt;}   /**< Set list of outgoing interfaces*/
        void addOutInt(outInterface outInt) {this->outInt.push_back(outInt);}                                           /**< Add interface to list of outgoing interfaces*/
        void setRegStatus(int intId, RegisterState regState);                                                           /**< set register status to given interface*/
        RegisterState getRegStatus(int intId);

        bool isRpf(int intId){if (intId == inInt.intId) return true; else return false;}                                /**< Returns if given interface is RPF or not*/
        bool isOilistNull();                                                                                            /**< Returns true if list of outgoing interfaces is empty, otherwise false*/

        IPv4Address   getRP() const {return RP;}                            /**< Get RP IP address */
        PIMgrt*     getGrt() const {return grt;}                            /**< Get pointer to PimGraftTimer */
        PIMsat*     getSat() const {return sat;}                            /**< Get pointer to PimSourceActiveTimer */
        PIMsrt*     getSrt() const {return srt;}                            /**< Get pointer to PimStateRefreshTimer */
        PIMkat*     getKat() const {return kat;}                            /**< Get pointer to KeepAliveTimer */
        PIMrst*     getRst() const {return rst;}                            /**< Get pointer to RegisterStopTimer */
        PIMet*      getEt()  const {return et;}                             /**< Get pointer to ExpiryTimer */
        PIMjt*      getJt()  const {return jt;}                             /**< Get pointer to JoinTimer */
        std::vector<flag> getFlags() const {return flags;}                  /**< Get list of route flags */

        // get incoming interface
        inInterface     getInInt() const {return inInt;}                    /**< Get incoming interface*/
        InterfaceEntry* getInIntPtr() const {return inInt.intPtr;}          /**< Get pointer to incoming interface*/
        int             getInIntId() const {return inInt.intId;}            /**< Get ID of incoming interface*/
        IPv4Address       getInIntNextHop() const {return inInt.nextHop;}     /**< Get IP address of next hop for incoming interface*/

        // get outgoing interface
        InterfaceVector getOutInt() const {return outInt;}                  /**< Get list of outgoing interfaces*/
        outInterface    getOutIntByIntId(int intId);                        /**< Get outgoing interface with given interface ID*/
        int             getOutIdByIntId(int intId);                         /**< Get sequence number of outgoing interface with given interface ID*/

        void            setOutShowIntStatus(bool status) {showOutInt = status;}
        bool            getOutShowIntStatus() const {return showOutInt;}

        simtime_t getInstallTime() const {return installtime;}
        void setInstallTime(simtime_t time) {installtime = time;}
        void setSequencenumber(int i){sequencenumber =i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};

#endif /* ANSAIPV4ROUTE_H_ */
