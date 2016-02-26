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
* @file BabelBuffer.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Buffer header file
* @detail Represents buffer used for sending messages
*/
#ifndef BABELBUFFER_H_
#define BABELBUFFER_H_

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelInterfaceTable.h"
#include "ansa/routing/babel/BabelFtlv.h"
namespace inet {

class INET_API BabelBuffer : public cObject
{
  protected:
    L3Address dst;
    BabelInterface *outIface;
    std::vector<BabelFtlv *> tlvs;
    Babel::BabelTimer *flushTimer;

  public:
    BabelBuffer():
        outIface(NULL),
        flushTimer(NULL)
        {};

    BabelBuffer(L3Address da, BabelInterface *oi, Babel::BabelTimer *ft):
        dst(da),
        outIface(oi),
        flushTimer(ft)
        {
            ASSERT(oi != NULL);
            ASSERT(ft != NULL);
        };

    virtual ~BabelBuffer();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelBuffer& buff);

    const L3Address& getDst() const {return dst;}
    void setDst(const L3Address& d) {dst = d;}

    Babel::BabelTimer* getFlushTimer() const {return flushTimer;}
    void setFlushTimer(Babel::BabelTimer* ft) {flushTimer = ft;}

    BabelInterface* getOutIface() const {return outIface;}
    void setOutIface(BabelInterface* oi) {outIface = oi;}


    std::vector<BabelFtlv *>::iterator tlvsBegin() {return tlvs.begin();}
    std::vector<BabelFtlv *>::const_iterator tlvsBegin() const {return tlvs.begin();}
    std::vector<BabelFtlv *>::iterator tlvsEnd() {return tlvs.end();}
    std::vector<BabelFtlv *>::const_iterator tlvsEnd() const {return tlvs.end();}
    size_t tlvsSize() const {return tlvs.size();}
    std::vector<BabelFtlv *>::iterator eraseTlv(std::vector<BabelFtlv *>::iterator it) {return tlvs.erase(it);}
    void addTlv(BabelFtlv *tlv) {tlvs.push_back(tlv);}
    std::vector<BabelFtlv *>::iterator getSimilarUpdateTlv(L3Address nh, Babel::rid origin);
    bool containTlv(uint8_t tlvtype) const;

};
}
#endif /* BABELBUFFER_H_ */
