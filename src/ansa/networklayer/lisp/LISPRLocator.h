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

#ifndef LISPRLOCATOR_H_
#define LISPRLOCATOR_H_

#include "IPvXAddress.h"
#include "LISPCommon.h"
#include <sstream>

class LISPRLocator {
  public:
    enum LocatorState {DOWN, ADMIN_DOWN, UP};

    bool operator== (const LISPRLocator& other) const;

    LISPRLocator();
    LISPRLocator(const char* addr);
    LISPRLocator(const char* addr, const char* prio, const char* wei);
    virtual ~LISPRLocator();

    std::string info() const;

    const IPvXAddress& getRloc() const;
    void setRloc(const IPvXAddress& rloc);
    LocatorState getState() const;
    std::string getStateString() const;
    void setState(LocatorState state);
    unsigned char getPriority() const;
    void setPriority(unsigned char priority);
    unsigned char getWeight() const;
    void setWeight(unsigned char weight);
    unsigned char getMpriority() const;
    void setMpriority(unsigned char mpriority);
    unsigned char getMweight() const;
    void setMweight(unsigned char mweight);

  private:
    IPvXAddress rloc;
    LocatorState state;
    unsigned char priority;
    unsigned char weight;
    unsigned char mpriority;
    unsigned char mweight;
};

class TLocator {
  public:
    bool LocalLocBit;                   // local locator Bit
    bool piggybackBit;                  // piggyback Bit
    bool RouteRlocBit;                  // route RLOC Bit
    LISPRLocator RLocator;              //<-- see LISP class struct

    std::string info() const;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPRLocator& locator);
std::ostream& operator<< (std::ostream& os, const TLocator& tloc);

#endif /* LISPRLOCATOR_H_ */
