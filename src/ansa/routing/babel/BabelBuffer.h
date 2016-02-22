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

class BabelBuffer : public cObject
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
