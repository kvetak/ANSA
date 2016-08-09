/*
 * NET.h
 *
 *  Created on: Aug 8, 2016
 *      Author: imarek
 */

#ifndef ANSA_NETWORKLAYER_CLNS_NET_H_
#define ANSA_NETWORKLAYER_CLNS_NET_H_

#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"


namespace inet {

class NET {
private:
    unsigned char* areaID;
    unsigned char* systemID;
    unsigned char* nsel; //this field is probably not part of NET, but part of NSAP (still confused)
public:
    NET();
    NET(std::string net);
    virtual ~NET();
};

}//end of namespace inet

#endif /* ANSA_NETWORKLAYER_CLNS_NET_H_ */
