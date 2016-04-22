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

#ifndef GLBPMESSAGE_H_
#define GLBPMESSAGE_H_

#include "inet/common/INETDefs.h"
//#include "inet/networklayer/contract/INetworkDatagram.h"
#include "ansa/networklayer/glbp/GLBPMessage_m.h"

namespace inet {

class GLBPMessage : public GLBPMessage_Base//, public INetworkDatagram
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
       * getter/setter for totalLength field in datagram
       * if set to -1, then getter returns getByteLength()
       */
//      int getTotalLengthField() const override;

//      /**
//       * Returns bits 0-5 of the Type of Service field, a value in the 0..63 range
//       */
//      virtual int getDiffServCodePoint() const override { return getTypeOfService() & 0x3f; }

//      /**
//       * Sets bits 0-5 of the Type of Service field; expects a value in the 0..63 range
//       */
//      virtual void setDiffServCodePoint(int dscp) override { setTypeOfService((getTypeOfService() & 0xc0) | (dscp & 0x3f)); }

//      /**
//       * Returns bits 6-7 of the Type of Service field, a value in the range 0..3
//       */
//      virtual int getExplicitCongestionNotification() const override { return (getTypeOfService() >> 6) & 0x03; }

//      /**
//       * Sets bits 6-7 of the Type of Service; expects a value in the 0..3 range
//       */
//      virtual void setExplicitCongestionNotification(int ecn) override { setTypeOfService((getTypeOfService() & 0x3f) | ((ecn & 0x3) << 6)); }

      /**
       * Returns the number of extension headers in this datagram
       */
      virtual unsigned int getOptionArraySize() const { return TLV_var.size(); }

      /**
       * Returns the kth extension header in this datagram
       */
      virtual TLVOptionBase& getTlvOption(unsigned int k) { return *check_and_cast<TLVOptionBase *>(&(TLV_var.at(k))); }
      virtual const TLVOptionBase& getTlvOption(unsigned int k) const { return const_cast<GLBPMessage*>(this)->getTlvOption(k); }

      /**
       * Returns the TLVOptionBase of the specified type,
       * or nullptr. If index is 0, then the first, if 1 then the
       * second option is returned.
       */
      virtual TLVOptionBase *findOptionByType(short int optionType, int index = 0);

      /**
       * Adds an TLVOptionBase to the datagram.
       * default atPos means add to the end.
       */
      virtual void addOption(TLVOptionBase *opt, int atPos = -1);

//      /**
//       * Calculates the length of the IPv6 header plus the extension
//       * headers.
//       */
//      virtual int calculateHeaderByteLength() const;


//      virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddress()); }
//      virtual void setSourceAddress(const L3Address& address) override { setSrcAddress(address.toIPv4()); }
//      virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddress()); }
//      virtual void setDestinationAddress(const L3Address& address) override { setDestAddress(address.toIPv4()); }
//      virtual int getTransportProtocol() const override { return GLBPMessage_Base::getTransportProtocol(); }
//      virtual void setTransportProtocol(int protocol) override { GLBPMessage_Base::setTransportProtocol(protocol); }

};

} /* namespace inet */

#endif /* GLBPMESSAGE_H_ */
