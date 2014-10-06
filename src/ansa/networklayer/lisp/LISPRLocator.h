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

#ifndef LISPRLOCATOR_H_
#define LISPRLOCATOR_H_

#include "IPvXAddress.h"
#include "LISPCommon.h"

class LISPRLocator {
  public:
    enum LocatorState {DOWN, ADMIN_DOWN, UP};

    bool operator== (const LISPRLocator& other) const;
    bool operator== (const LISPRLocator& other);

    LISPRLocator();
    LISPRLocator(const char* addr);
    LISPRLocator(const char* addr, const char* prio, const char* wei, bool loca);
    virtual ~LISPRLocator();

    std::string info() const;

    const IPvXAddress& getRlocAddr() const;
    void setRlocAddr(const IPvXAddress& rloc);
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
    bool isLocal() const;
    void setLocal(bool local);

    void updateRlocator(const LISPRLocator& rloc);

  private:
    IPvXAddress rloc;
    LocatorState state;
    unsigned char priority;
    unsigned char weight;
    //TODO: Vesely - Multicast support
    unsigned char mpriority;
    unsigned char mweight;
    bool local;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPRLocator& locator);

#endif /* LISPRLOCATOR_H_ */
