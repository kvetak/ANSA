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
#include <sstream>

extern const int LISP_DEFAULT_PRIORITY;
extern const int LISP_DEFAULT_WEIGHT;
extern const int LISP_DEFAULT_RLOCSTATE;

class LISPRLocator {
  public:
    enum LocatorState {DOWN, ADMIN_DOWN, UP};

    bool operator== (const LISPRLocator& other) const;

    LISPRLocator();
    LISPRLocator(const char* addr);
    LISPRLocator(const char* addr, const char* prio, const char* wei);
    LISPRLocator(IPvXAddress addr);
    LISPRLocator(IPvXAddress addr, short prio, short wei);
    LISPRLocator(IPvXAddress addr, LocatorState state, short pri, short wei);
    virtual ~LISPRLocator();

    std::string info() const;

    short getPriority() const;
    void setPriority(short priority);
    const IPvXAddress& getRloc() const;
    void setRloc(const IPvXAddress& rloc);
    LocatorState getState() const;
    void setState(LocatorState state);
    short getWeight() const;
    void setWeight(short weight);

  private:
    IPvXAddress rloc;
    LocatorState state;
    short priority;
    short weight;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPRLocator& locator);

#endif /* LISPRLOCATOR_H_ */
