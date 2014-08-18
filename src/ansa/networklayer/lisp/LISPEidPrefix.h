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

#ifndef LISPEIDPREFIX_H_
#define LISPEIDPREFIX_H_

#include "IPvXAddress.h"

#define LISP_DEFAULT_LENGTH     0

class LISPEidPrefix {
  public:
    LISPEidPrefix();
    LISPEidPrefix(const char* address, const char* length);
    LISPEidPrefix(IPvXAddress addr, short len);
    virtual ~LISPEidPrefix();

    bool operator== (const LISPEidPrefix& other) const;

    std::string info() const;

    const IPvXAddress& getEid() const;
    void setEid(const IPvXAddress& eid);
    short getEidLen() const;
    void setEidLen(short eidLen);

  private:
    IPvXAddress eid;
    short eidLen;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPEidPrefix& ep);

#endif /* LISPEIDPREFIX_H_ */
