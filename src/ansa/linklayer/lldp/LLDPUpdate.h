//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file LLDPUpdate.h
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef LLDPUPDATE_H_
#define LLDPUPDATE_H_

#include "inet/common/INETDefs.h"
#include "ansa/linklayer/lldp/LLDPUpdate_m.h"

namespace inet {


class INET_API LLDPUpdate : public LLDPUpdate_Base
{
  private:
    void copy(const LLDPUpdate& other){};
    void clean();

  public:
    LLDPUpdate(const char *name=nullptr, int kind=0) : LLDPUpdate_Base(name,kind) {}
    LLDPUpdate(const LLDPUpdate& other) : LLDPUpdate_Base(other) {copy(other);}
    LLDPUpdate& operator=(const LLDPUpdate& other) {if (this==&other) return *this; LLDPUpdate_Base::operator=(other); copy(other); return *this;}
    virtual LLDPUpdate *dup() const {return new LLDPUpdate(*this);}

    /**
     * Returns number of TLV in message.
     */
    virtual unsigned int getOptionArraySize() const { return options.size(); }
    /**
     * Get length of the specified option.
     */
    short getOptionLength(TlvOptionBase *opt);

    // getters
    std::string getMsap();
    uint16_t getTtl();
    const char *getChassisId();
    const char *getPortId();

    /**
     * Set length of the specified option.
     */
    virtual void setOptionLength(TlvOptionBase *opt);

    /**
     * Returns option
     */
    virtual TlvOptionBase& getOption(unsigned int k) { return *check_and_cast<TlvOptionBase *>(&(options.at(k))); }
    virtual const TlvOptionBase& getOption(unsigned int k) const { return const_cast<LLDPUpdate*>(this)->getOption(k); }

    /**
     * Returns the TlvOptionBase of the specified type,
     * or nullptr. If index is 0, then the first, if 1 then the
     * second option is returned.
     */
    virtual TlvOptionBase *findOptionByType(short int optionType, int index = 0);

    /**
     * Adds an TlvOptionBase to the update.
     * default atPos means add to the end.
     */
    virtual void addOption(TlvOptionBase *opt, int atPos = -1);

};
} /* namespace inet */

#endif /* LLDPUPDATE_H_ */
