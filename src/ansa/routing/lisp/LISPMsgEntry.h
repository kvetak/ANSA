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
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */


#ifndef LISPMSGENTRY_H_
#define LISPMSGENTRY_H_

#include <omnetpp.h>
#include "ansa/routing/lisp/LISPCommon.h"

namespace inet {

class LISPMsgEntry {
  public:
    enum EMsgType {
        UNKNOWN = 0,
        REQUEST,
        REPLY,
        NEGATIVE_REPLY,
        REGISTER,
        NOTIFY,
        ENCAPSULATED_REQUEST,
        RLOC_PROBE,
        RLOC_PROBE_REPLY,
        CACHE_SYNC,
        CACHE_SYNC_ACK,
        DATA
    };

    LISPMsgEntry(LISPMsgEntry::EMsgType ntyp, unsigned long nnonce, L3Address addr, simtime_t processed, bool fl, int64 siz);
    virtual ~LISPMsgEntry();

    bool operator== (const LISPMsgEntry& other) const;

    std::string info() const;

    unsigned long getNonce() const;
    void setNonce(unsigned long nonce);
    const simtime_t& getProcessedAt() const;
    void setProcessedAt(const simtime_t& processedAt);
    LISPMsgEntry::EMsgType getType() const;
    void setType(EMsgType type);
    std::string getTypeString() const;
    const L3Address& getAddress() const;
    void setAddress(const L3Address& destination);
    bool isFlag() const;
    void setFlag(bool flag);

  private:
    EMsgType type;
    unsigned long nonce;
    L3Address address;
    simtime_t processedAt;
    bool flag; //received = 0, sent = 1;
    int64 msgsize;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPMsgEntry& entry);

}
#endif /* LISPMSGENTRY_H_ */
