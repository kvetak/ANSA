//
// Copyright (C) 2013, 2014 Brno University of Technology
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
/**
 * @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
 */

#ifndef LISPEIDPREFIX_H_
#define LISPEIDPREFIX_H_

#include "IPvXAddress.h"
#include "LISPCommon.h"

class LISPEidPrefix {

  public:

    LISPEidPrefix();
    LISPEidPrefix(const char* address, const char* length);
    LISPEidPrefix(IPvXAddress address, unsigned char length);
    virtual ~LISPEidPrefix();

    bool operator== (const LISPEidPrefix& other) const;

    std::string info() const;

    const IPvXAddress& getEidAddr() const;
    void setEidAddr(const IPvXAddress& eid);
    unsigned char getEidLength() const;
    void setEidLength(unsigned char eidLen);
    LISPCommon::Afi getEidAfi() const;

    bool isComponentOf(const LISPEidPrefix& coarserEid) const;

  private:
    IPvXAddress eid;
    unsigned char eidLen;
    LISPCommon::Afi eidAfi;

};


//Free function
std::ostream& operator<< (std::ostream& os, const LISPEidPrefix& ep);



#endif /* LISPEIDPREFIX_H_ */
