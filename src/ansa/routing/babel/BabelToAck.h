// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/**
* @file BabelToAck.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel ToACK header file
* @detail Represents data structure for saving messages waiting for Acknowledgment
*/

#ifndef BABELTOACK_H_
#define BABELTOACK_H_

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelInterfaceTable.h"

namespace inet {

class INET_API BabelToAck : public cObject
{
  protected:
    uint16_t nonce;
    std::vector<L3Address> dstNodes;
    int resendNum;
    Babel::BabelTimer *resendTimer;

    L3Address dst;
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

    BabelToAck(uint16_t n, int rn, Babel::BabelTimer *rt, L3Address d, BabelInterface *oi, BabelMessage *m):
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

    const L3Address& getDst() const {return dst;}
    void setDst(const L3Address& d) {dst = d;}

    BabelInterface* getOutIface() const {return outIface;}
    void setOutIface(BabelInterface* oi) {outIface = oi;}

    BabelMessage* getMsg() const {return msg;}
    void setMsg(BabelMessage* m) {msg = m;}

    void addDstNode(L3Address dn);
    void removeDstNode(L3Address dn);
    size_t dstNodesSize() {return dstNodes.size();}

};
}
#endif /* BABELTOACK_H_ */
