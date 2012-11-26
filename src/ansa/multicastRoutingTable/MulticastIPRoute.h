/**
 * @file MulticastIPRoute.h
 * @brief File contains implementation of multicast route.
 * @author Veronika Rybova
 * @date 10.10.2011
 */

#ifndef MULTICASTIPROUTE_H_
#define MULTICASTIPROUTE_H_

#include <omnetpp.h>
#include "IPAddress.h"
#include "InterfaceEntry.h"
#include "PIMTimer_m.h"

/**
 * Route flags. Added to each route.
 */
enum flag
{
	D,				/**< Dense */
	S,				/**< Sparse */
	C,				/**< Connected */
	P,				/**< Pruned */
	A				/**< Source is directly connected */
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
 * @brief Structure of incoming interface.
 * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
 */
struct inInterface
{
	InterfaceEntry 			*intPtr; 			/**< Pointer to interface */
	int						intId;				/**< Interface ID */
	IPAddress				nextHop;			/**< RF neighbor */
};

/**
 * @brief Structure of outgoing interface.
 * @details E.g.: Ethernet0, Forward/Sparse, 5:29:15/0:02:57
 */
struct outInterface
{
	InterfaceEntry 			*intPtr; 			/**< Pointer to interface */
	int						intId;				/**< Interface ID */
	intState				forwarding;			/**< Forward or Pruned */
	intState				mode;				/**< Dense, Sparse, ... */
	PIMpt 					*pruneTimer;		/**< Pointer to PIM Prune Timer*/
	AssertState				assert;				/**< Assert state. */
};

/**
 * Vector of outgoing interfaces.
 */
typedef std::vector<outInterface> InterfaceVector;


/**
 * @brief Class represents one entry of MulticastRoutingTable.
 */
class INET_API MulticastIPRoute : public cPolymorphic
{
  private:
    IPAddress 					source;     			/**< Source of multicast */
    IPAddress 					group;					/**< Multicast group */
    IPAddress 					RP;  					/**< Randevous point */
    std::vector<flag>			flags;					/**< Route flags */
    // timers
    PIMgrt						*grt;					/**< Pointer to Graft Retry Timer*/
    PIMsat						*sat; 					/**< Pointer to Source Active Timer*/
    PIMsrt						*srt;					/**< Pointer to State Refresh Timer*/
    // interfaces
    inInterface  				inInt;					/**< Incoming interface */
    InterfaceVector				outInt;					/**< Outgoing interface */



    //Originated from destination.Ensures loop freeness.
    unsigned int sequencenumber;
    //Time of routing table entry creation
    simtime_t installtime;


  private:
    MulticastIPRoute& operator=(const MulticastIPRoute& obj);

  public:
    MulticastIPRoute();													/**< Set all pointers to null */
    virtual ~MulticastIPRoute() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    void setSource(IPAddress source)  {this->source = source;}			/**< Set multicast source IP address */
    void setGroup(IPAddress group)  {this->group = group;}				/**< Set multicast group IP address */
    void setRP(IPAddress RP)  {this->RP = RP;}							/**< Set RP IP address */
    void setGrt (PIMgrt *grt)  {this->grt = grt;}						/**< Set pointer to PimGraftTimer */
    void setSat (PIMsat *sat)  {this->sat = sat;}						/**< Set pointer to PimSourceActiveTimer */
    void setSrt (PIMsrt *srt)  {this->srt = srt;}						/**< Set pointer to PimStateRefreshTimer */

    void setFlags(std::vector<flag> flags)  {this->flags = flags;}		/**< Set vector of flags (flag) */
    bool isFlagSet(flag fl);											/**< Returns if flag is set to entry or not*/
    void addFlag(flag fl);											 	/**< Add flag to ineterface */
    void removeFlag(flag fl);											/**< Remove flag from ineterface */

    void setInInt(InterfaceEntry *interfacePtr, int intId, IPAddress nextHop) {this->inInt.intPtr = interfacePtr; this->inInt.intId = intId; this->inInt.nextHop = nextHop;}	/**< Set information about incoming interface*/
    void setInInt(inInterface inInt) {this->inInt = inInt;}															/**< Set incoming interface*/

    void setOutInt(InterfaceVector outInt) {EV << "MulticastIPRoute: New OutInt" << endl; this->outInt = outInt;}	/**< Set list of outgoing interfaces*/
    void addOutInt(outInterface outInt) {this->outInt.push_back(outInt);}											/**< Add interface to list of outgoing interfaces*/

    bool isRpf(int intId){if (intId == inInt.intId) return true; else return false;}								/**< Returns if given interface is RPF or not*/
    bool isOilistNull();																							/**< Returns true if list of outgoing interfaces is empty, otherwise false*/


    IPAddress 	getSource() const {return source;}						/**< Get multicast source IP address */
    IPAddress 	getGroup() const {return group;}						/**< Get multicast group IP address */
    IPAddress 	getRP() const {return RP;}								/**< Get RP IP address */
    PIMgrt*		getGrt() const {return grt;}							/**< Get pointer to PimGraftTimer */
    PIMsat*		getSat() const {return sat;}							/**< Get pointer to PimSourceActiveTimer */
    PIMsrt*		getSrt() const {return srt;}							/**< Get pointer to PimStateRefreshTimer */
    std::vector<flag> getFlags() const {return flags;}					/**< Get list of route flags */

    // get incoming interface
    inInterface 	getInInt() const {return inInt;}					/**< Get incoming interface*/
    InterfaceEntry* getInIntPtr() const {return inInt.intPtr;}			/**< Get pointer to incoming interface*/
    int				getInIntId() const {return inInt.intId;}			/**< Get ID of incoming interface*/
    IPAddress		getInIntNextHop() const {return inInt.nextHop;}		/**< Get IP address of next hop for incoming interface*/

    // get outgoing interface
    InterfaceVector getOutInt() const {return outInt;}					/**< Get list of outgoing interfaces*/
    outInterface 	getOutIntByIntId(int intId);						/**< Get outgoing interface with given interface ID*/
    int 			getOutIdByIntId(int intId);							/**< Get sequence number of outgoing interface with given interface ID*/


    simtime_t getInstallTime() const {return installtime;}
    void setInstallTime(simtime_t time) {installtime = time;}
    void setSequencenumber(int i){sequencenumber =i;}
    unsigned int getSequencenumber() const {return sequencenumber;}

};


#endif /* MULTICASTIPROUTE_H_ */
