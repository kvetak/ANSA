/*
 * stpXMLparser.h
 *
 *  Created on: 17.5.2011
 *      Author: aranel
 */

#ifndef STPXMLPARSER_H_
#define STPXMLPARSER_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include "stp.h"

class stpXMLparser {
public:
	stpXMLparser(Stp *);
	virtual ~stpXMLparser();

	bool parse(const char *, const char *);

	void parseSTP(cXMLElement * node);

private:
	Stp * stp;

};

#endif /* STPXMLPARSER_H_ */
