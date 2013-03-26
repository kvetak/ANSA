/*
 * vlanTableXMLparser.h
 *
 *  Created on: 16.12.2010
 *      Author: xkraus00
 */

#ifndef VLANTABLEXMLPARSER_H_
#define VLANTABLEXMLPARSER_H_

#include <omnetpp.h>
#include <string>
#include "vlanTable.h"


class INET_API VLANTableXMLparser {

public:
	VLANTableXMLparser(VLANTable *);
	virtual ~VLANTableXMLparser();

	/* Main parsing function */
	bool parse(const char *, const char *);

	void parseInterfaces(cXMLElement * node);
	/* parse Port (portVID, etc) info */
	void parseVIDPort(cXMLElement * node);

	/* parsing VLANs section */
	void parseVLANs(cXMLElement * node);
	/* parse particular VLAN info */
	void parseVLAN(cXMLElement * node);

	/* parse XML tag written as list  e.g.  <Tagged>1,2,3,4,5,6</Tagged>  */
	void parseList(std::vector<int>&, const char *);


private:
	VLANTable *table;


};

#endif /* VLANTABLEXMLPARSER_H_ */
