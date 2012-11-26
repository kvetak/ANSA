/**
 * @file PimNeighborTable.h
 * @date 19.3.2012
 * @author: Veronika Rybova
 * @brief File implements table of PIM neighbors.
 * @details Table of neighbors contain information about all PIM neghbor routers which has
 *  also configured PIM protocol. Information about neighbors are obtained from
 *  Hello messages.
 */

#ifndef PIMNEIGHBOR_H_
#define PIMNEIGHBOR_H_

#include <omnetpp.h>
#include "AnsaInterfaceTable.h"
#include "PIMTimer_m.h"



/**
 * @brief  Class represents one entry of PimNeighborTable.
 * @details Structure PIM neighbor with info about interface, IP address of neighbor
 * link to Neighbor Livness Timer and PIM version. The class contains
 * methods to work with items of structure.
 */
class INET_API PimNeighbor: public cPolymorphic
{
	protected:
		int					id;					/**< Unique identifier of entry. */
		int 				intID;          	/**< Identification of interface. */
		InterfaceEntry 		*intPtr;			/**< Link to interface table entry. */
		IPAddress 			addr; 				/**< IP address of neighbor. */
		int					ver;				/**< PIM version. */
		PIMnlt 				*nlt;				/**< Pointer to Neighbor Livness Timer. */

	public:
		PimNeighbor(){};
	    virtual ~PimNeighbor() {};
	    virtual std::string info() const;

	    // set methods
	    void setId(int id)  {this->id = id;}										/**< Set unique identifier of entry. */
	    void setInterfaceID(int intID)  {this->intID = intID;}						/**< Set interface ID. */
	    void setInterfacePtr(InterfaceEntry *intPtr)  {this->intPtr = intPtr;}		/**< Set pointer to interface. */
	    void setAddr(IPAddress addr) {this->addr = addr;}							/**< Set IP address of neighbor. */
	    void setVersion(int ver) {this->ver = ver;}									/**< Set PIM version (from Hello msg). */
	    void setNlt(PIMnlt *nlt) {this->nlt = nlt;}									/**< Set pointer to NeighborLivenessTimer. */


	    // get methods
	    int getId() const {return id;}												/**< Get unique identifier of entry. */
	    int getInterfaceID() const {return intID;}									/**< Get interface ID. */
	    InterfaceEntry *getInterfacePtr() const {return intPtr;}					/**< Get pointer to interface. */
	    IPAddress getAddr() const {return addr;}									/**< Get IP address of neighbor. */
	    int getVersion() const {return ver;}										/**< Get PIM version. */
	    PIMnlt *getNlt() const {return nlt;}										/**< Get pointer to NeighborLivenessTimer. */
};

/**
 * @brief Class represents Pim Neighbor Table.
 * @details Table is list of PimNeighbor and class contains methods to work with them.
 */
class INET_API PimNeighborTable: public cSimpleModule
{
	protected:
		int							id;				/**< Counter of PimNeighbor IDs*/
		std::vector<PimNeighbor>	nt;				/**< List of PIM neighbors (show ip pim neighbor) */

	public:
		PimNeighborTable(){};
		virtual ~PimNeighborTable(){};

		virtual PimNeighbor *getNeighbor(int k){return &this->nt[k];}				/**< Get k-th entry in the table */
		virtual void addNeighbor(PimNeighbor entry){entry.setId(id); this->nt.push_back(entry); id++;}	/**< Add new entry to the table*/
		virtual bool deleteNeighbor(int id);
		virtual int getNumNeighbors() {return this->nt.size();}						/**< Get number of entries in the table */
		virtual void printPimNeighborTable();
		virtual std::vector<PimNeighbor> getNeighborsByIntID(int intID);
		virtual PimNeighbor *getNeighborsByID(int id);
		virtual int getIdCounter(){return this->id;}								/**< Get counter of entry IDs */
		virtual bool isInTable(PimNeighbor entry);
		virtual PimNeighbor *findNeighbor(int intId, IPAddress addr);
		virtual int getNumNeighborsOnInt(int intId);

	protected:
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
};

/**
 * @brief Class gives access to the PimNeighborTable.
 */
class INET_API PimNeighborTableAccess : public ModuleAccess<PimNeighborTable>
{
	public:
		PimNeighborTableAccess() : ModuleAccess<PimNeighborTable>("PimNeighborTable") {}
};

#endif /* PIMNEIGHBOR_H_ */
