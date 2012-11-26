/*
 * PimInterfaces.h
 *
 *  Created on: 19.3.2012
 *      Author: Haczek
 */

#ifndef PIMINTERFACES_H_
#define PIMINTERFACES_H_

#include <omnetpp.h>
#include "AnsaInterfaceTableAccess.h"
#include "AnsaInterfaceTable.h"


/**
 * PIM modes.
 */
enum PIMmode {
    Dense = 1,
    Sparse = 2
};

class INET_API PimInterface: public cPolymorphic
{
	protected:
		int 			intID;          /**< Identification of interface. */
		InterfaceEntry *intPtr;			/**< Link to interface table entry. */
		PIMmode			mode;			/**< Type of mode. */

	public:
		PimInterface(){};
	    virtual ~PimInterface() {};
	    virtual std::string info() const;
	    //friend std::ostream& operator<<(std::ostream& os, const PimInterface& e);
	    //virtual std::string info() const;
	    //virtual std::string detailedInfo() const;

	    void setInterfaceID(int iftID)  {this->intID = iftID;}
	    void setInterfacePtr(InterfaceEntry *intPtr)  {this->intPtr = intPtr;}
	    void setMode(PIMmode mode) {this->mode = mode;}

	    int getInterfaceID() const {return intID;}
	    InterfaceEntry *getInterfacePtr() const {return intPtr;}
	    PIMmode getMode() const {return mode;}
};

class INET_API PimInterfaces: public cSimpleModule
{
	protected:
		std::vector<PimInterface>	pimIft;			/**< List of PIM interfaces. */

	public:
		PimInterfaces(){};
		virtual ~PimInterfaces(){};

		virtual PimInterface *getInterface(int k){return &this->pimIft[k];}
		virtual void addInterface(const PimInterface entry){this->pimIft.push_back(entry);}
		//virtual bool deleteInterface(const PimInterface *entry){};
		virtual int getNumInterface() {return this->pimIft.size();}
		virtual void printPimInterfaces();
		virtual PimInterface *getInterfaceByIntID(int intID);

	protected:
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
};

/**
 * Gives access to the PimInterfaces.
 */
class INET_API PimInterfacesAccess : public ModuleAccess<PimInterfaces>
{
    public:
	PimInterfacesAccess() : ModuleAccess<PimInterfaces>("PimInterfaces") {}
};

#endif /* PIMINTERFACES_H_ */
