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
* @author Jan Holusa
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef GLBPMESSAGE_H_
#define GLBPMESSAGE_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/glbp/GLBPMessage_m.h"

namespace inet {
/**
* @file GLBPMessage.h
* @author Jan Holusa
* @brief Extended message file, needed because of TLV
*/
class GLBPMessage : public GLBPMessage_Base
{
    private:
      void copy(const GLBPMessage& other);
      void clean();

    public:
      GLBPMessage(const char *name = nullptr, int kind = 0) : GLBPMessage_Base(name, kind) {}
      GLBPMessage(const GLBPMessage& other) : GLBPMessage_Base(other) {}
      GLBPMessage& operator=(const GLBPMessage& other) { GLBPMessage_Base::operator=(other); return *this; }
      virtual GLBPMessage *dup() const override { return new GLBPMessage(*this); }

      /**
       * Returns the number of extension headers in this datagram
       */
      virtual unsigned int getOptionArraySize() const { return getTLV().size(); }

      /**
       * Returns the kth extension header in this datagram
       */
      virtual TlvOptionBase& getTlvOption(unsigned int k) { return *check_and_cast<TlvOptionBase *>(&(getTLV().at(k))); }
      virtual const TlvOptionBase& getTlvOption(unsigned int k) const { return const_cast<GLBPMessage*>(this)->getTlvOption(k); }

      /**
       * Returns the TlvOptionBase of the specified type,
       * or nullptr. If index is 0, then the first, if 1 then the
       * second option is returned.
       */
      virtual TlvOptionBase *findOptionByType(short int optionType, int index = 0);

      /**
       * Adds an TlvOptionBase to the datagram.
       * default atPos means add to the end.
       */
      virtual void addOption(TlvOptionBase *opt, int atPos = -1);
};

} /* namespace inet */

#endif /* GLBPMESSAGE_H_ */
