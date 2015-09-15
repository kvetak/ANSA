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
* @file BabelToAck.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel ToACK header file
* @detail Represents data structure for saving messages waiting for Acknowledgment
*/

#ifndef BABELTOACK_H_
#define BABELTOACK_H_

#include "ansa/applications/babel/BabelDef.h"
#include "ansa/applications/babel/BabelInterfaceTable.h"



class BabelToAck : public cObject
{
  protected:
    uint16_t nonce;
    std::vector<inet::L3Address> dstNodes;
    int resendNum;
    Babel::BabelTimer *resendTimer;

    inet::L3Address dst;
    BabelInterface *outIface;
    BabelMessage *msg;

  public:
    BabelToAck():
        nonce(0),
        resendNum(0),
        resendTimer(NULL),
        outIface(NULL),
        msg(NULL)
        {};

    BabelToAck(uint16_t n, int rn, Babel::BabelTimer *rt, inet::L3Address d, BabelInterface *oi, BabelMessage *m):
        nonce(n),
        resendNum(rn),
        resendTimer(rt),
        dst(d),
        outIface(oi),
        msg(m)
        {
            ASSERT(rt != NULL);
            ASSERT(oi != NULL);
            ASSERT(m != NULL);

            resendTimer->setContextPointer(this);
        };
    virtual ~BabelToAck();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelToAck& toack);

    uint16_t getNonce() const {return nonce;}
    void setNonce(uint16_t n) {nonce = n;}

    int getResendNum() const {return resendNum;}
    void setResendNum(int rn) {resendNum = rn;}
    int decResendNum() {return --resendNum;}

    Babel::BabelTimer* getResendTimer() const {return resendTimer;}
    void setResendTimer(Babel::BabelTimer* rt) {resendTimer = rt;}

    const inet::L3Address& getDst() const {return dst;}
    void setDst(const inet::L3Address& d) {dst = d;}

    BabelInterface* getOutIface() const {return outIface;}
    void setOutIface(BabelInterface* oi) {outIface = oi;}

    BabelMessage* getMsg() const {return msg;}
    void setMsg(BabelMessage* m) {msg = m;}

    void addDstNode(inet::L3Address dn);
    void removeDstNode(inet::L3Address dn);
    size_t dstNodesSize() {return dstNodes.size();}

};

#endif /* BABELTOACK_H_ */
