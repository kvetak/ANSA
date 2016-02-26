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
