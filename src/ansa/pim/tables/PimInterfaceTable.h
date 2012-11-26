/**
 * @file PimInterfaceTable.h
 * @date 19.3.2012
 * @author: Veronika Rybova
 * @brief File implements table of PIM interfaces.
 * @details PIM interface table contains information about all interfaces which are configured by
 *  PIM protocol. Information are obtained from configuration file.
 */

#ifndef PIMINTERFACES_H_
#define PIMINTERFACES_H_

#include <omnetpp.h>
#include "AnsaInterfaceTable.h"
#include "IPAddress.h"

/**
 * PIM modes configured on the interface.
 */
enum PIMmode
{
    Dense = 1,
    Sparse = 2
};

/**
 * @brief  Class represents one entry of PimInterfaceTable.
 * @details One entry contains interfaces ID, pointer to the interface, PIM mode and multicast
 * addresses assigned to the interface.
 */
class INET_API PimInterface: public cPolymorphic
{
	protected:
		int 					intID;          			/**< Identification of interface. */
		InterfaceEntry *		intPtr;						/**< Pointer to interface table entry. */
		PIMmode					mode;						/**< Type of mode. */
		std::vector<IPAddress> 	intMulticastAddresses;		/**< Multicast addresses assigned to interface. */

	public:
		PimInterface(){intPtr = NULL;};
	    virtual ~PimInterface() {};
	    virtual std::string info() const;

	    // set methods
	    void setInterfaceID(int iftID)  {this->intID = iftID;}							/**< Set identifier of interface. */
	    void setInterfacePtr(InterfaceEntry *intPtr)  {this->intPtr = intPtr;}			/**< Set pointer to interface. */
	    void setMode(PIMmode mode) {this->mode = mode;}									/**< Set PIM mode configured on the interface. */

	    //get methods
	    int getInterfaceID() const {return intID;}													/**< Get identifier of interface. */
		InterfaceEntry *getInterfacePtr() const {return intPtr;}									/**< Get pointer to interface. */
		PIMmode getMode() const {return mode;}														/**< Get PIM mode configured on the interface. */
		std::vector<IPAddress> getIntMulticastAddresses() const {return intMulticastAddresses;}		/**< Get list of multicast addresses assigned to the interface. */

	    // methods for work with vector "intMulticastAddresses"
	    void setIntMulticastAddresses(std::vector<IPAddress> intMulticastAddresses)  {this->intMulticastAddresses = intMulticastAddresses;} 	/**< Set multicast addresses to the interface. */
	    void addIntMulticastAddress(IPAddress addr)  {this->intMulticastAddresses.push_back(addr);}												/**< Add multicast address to the interface. */
	    void removeIntMulticastAddress(IPAddress addr);
	    bool isLocalIntMulticastAddress (IPAddress addr);
	    std::vector<IPAddress> deleteLocalIPs(std::vector<IPAddress> multicastAddr);
};


/**
 * @brief Class represents Pim Interface Table.
 * @brief It is vector of PimInterface. Class contains methods to work with the table.
 */
class INET_API PimInterfaceTable: public cSimpleModule
{
	protected:
		std::vector<PimInterface>	pimIft;					/**< List of PIM interfaces. */

	public:
		PimInterfaceTable(){};
		virtual ~PimInterfaceTable(){};

		virtual PimInterface *getInterface(int k){return &this->pimIft[k];}						/**< Get pointer to entry of PimInterfaceTable from the object. */
		virtual void addInterface(const PimInterface entry){this->pimIft.push_back(entry);}		/**< Add entry to PimInterfaceTable. */
		//virtual bool deleteInterface(const PimInterface *entry){};
		virtual int getNumInterface() {return this->pimIft.size();}								/**< Returns number of entries in PimInterfaceTable. */
		virtual void printPimInterfaces();
		virtual PimInterface *getInterfaceByIntID(int intID);									/**< Returns entry from PimInterfaceTable with given interface ID. */

	protected:
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
};

/**
 * @brief Class gives access to the PimInterfaceTable.
 */
class INET_API PimInterfaceTableAccess : public ModuleAccess<PimInterfaceTable>
{
	private:
		PimInterfaceTable *p;

	public:
	PimInterfaceTableAccess() : ModuleAccess<PimInterfaceTable>("PimInterfaceTable") {p=NULL;}

	virtual PimInterfaceTable *getMyIfExists()
	{
		if (!p)
		{
			cModule *m = findModuleWherever("PimInterfaceTable", simulation.getContextModule());
			p = dynamic_cast<PimInterfaceTable*>(m);
		}
		return p;
	}
};

/**
 * @brief Class is needed by notification about new multicast addresses on interface.
 * @details If you do not use notification board, you probably do not need this class.
 * The problem is that method fireChangeNotification
 * needs object as the second parameter.
 */
class addRemoveAddr : public cPolymorphic
{
	protected:
		std::vector<IPAddress> addr;						/**< Vector of added or removed addresses. */
		PimInterface *pimInt;								/**< Pointer to interface. */

	public:
		addRemoveAddr(){};
		virtual ~addRemoveAddr() {};
		virtual std::string info() const
		{
			std::stringstream out;
			for (unsigned int i = 0; i < addr.size(); i++)
				out << addr[i] << endl;
			return out.str();
		}

		void setAddr(std::vector<IPAddress> addr)  {this->addr = addr;} /**< Set addresses to the object. */
		void setInt(PimInterface *pimInt)  {this->pimInt = pimInt;}		/**< Set pointer to interface to the object. */
		std::vector<IPAddress> getAddr () {return this->addr;}			/**< Get addresses from the object. */
		int getAddrSize () {return this->addr.size();}					/**< Returns size of addresses vector. */
		PimInterface *getInt () {return this->pimInt;}					/**< Get pointer to interface from the object. */
};

#endif /* PIMINTERFACES_H_ */
