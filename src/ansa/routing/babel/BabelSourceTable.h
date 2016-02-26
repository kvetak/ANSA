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
* @file BabelSourceTable.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Source Table header file
* @detail Represents data structure for saving feasible distances
*/

#ifndef BABELSOURCETABLE_H_
#define BABELSOURCETABLE_H_

#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
namespace inet {
class INET_API BabelSource: public cObject
{
protected:
    Babel::netPrefix<L3Address> prefix;
    Babel::rid originator;
    Babel::routeDistance feasibleDistance;
    Babel::BabelTimer *gcTimer;

public:
    BabelSource():
    gcTimer(NULL)
    {}
    BabelSource(const Babel::netPrefix<L3Address>& pre,
                const Babel::rid& orig,
                const Babel::routeDistance& fd,
                Babel::BabelTimer *gct):
        prefix(pre),
        originator(orig),
        feasibleDistance(fd),
        gcTimer(gct)
        {
            if(gcTimer != NULL)
            {
                gcTimer->setContextPointer(this);
            }
        }

    virtual ~BabelSource();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelSource& bs);

    const Babel::netPrefix<L3Address>& getPrefix() const {return prefix;}
    void setPrefix(Babel::netPrefix<L3Address>& p) {prefix = p;}

    const Babel::rid& getOriginator() const {return originator;}
    void setOriginator(const Babel::rid& o) {originator = o;}

    Babel::routeDistance& getFDistance() {return feasibleDistance;}
    const Babel::routeDistance& getFDistance() const {return feasibleDistance;}
    void setFDistance(const Babel::routeDistance& fd) {feasibleDistance = fd;}

    Babel::BabelTimer* getGCTimer() const {return gcTimer;}
    void setGCTimer(Babel::BabelTimer* gct) {gcTimer = gct;}

    void resetGCTimer();
    void resetGCTimer(double delay);
    void deleteGCTimer();
};


class BabelSourceTable {
protected:
    std::vector<BabelSource *> sources;

public:
    BabelSourceTable() {}
    virtual ~BabelSourceTable();

    std::vector<BabelSource *>& getSources() {return sources;}

    BabelSource *findSource(const Babel::netPrefix<L3Address>& p, const Babel::rid& orig);
    BabelSource *addSource(BabelSource *source);
    void removeSource(BabelSource *source);
    void removeSources();
};
}
#endif /* BABELSOURCETABLE_H_ */
