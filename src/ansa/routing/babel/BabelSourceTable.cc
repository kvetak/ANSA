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
* @file BabelSourceTable.cc
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel Source Table
* @detail Represents data structure for saving feasible distances
*/

#include "ansa/routing/babel/BabelSourceTable.h"
namespace inet {
using namespace Babel;

BabelSource::~BabelSource()
{
    deleteGCTimer();
}

std::ostream& operator<<(std::ostream& os, const BabelSource& bs)
{
    return os << bs.str();
}

std::string BabelSource::str() const
{
    std::stringstream string;
#ifdef BABEL_DEBUG
    string << prefix;
    string << ", originator: " << originator;
    string << ", FD: " << feasibleDistance;
    if(gcTimer != NULL && gcTimer->isScheduled()) string << ", gc timeout at T=" << gcTimer->getArrivalTime();
#else
    string << prefix;
    string << " orig:" << originator;
    string << " FD:" << feasibleDistance;
#endif
    return string.str();
}


void BabelSource::resetGCTimer()
{
    ASSERT(gcTimer != NULL);

    resetTimer(gcTimer, defval::SOURCE_GC_INTERVAL);
}

void BabelSource::resetGCTimer(double delay)
{
    ASSERT(gcTimer != NULL);

    resetTimer(gcTimer, delay);
}

void BabelSource::deleteGCTimer()
{
    deleteTimer(&gcTimer);
}



BabelSourceTable::~BabelSourceTable()
{
    removeSources();
}

BabelSource *BabelSourceTable::findSource(const netPrefix<L3Address>& p, const rid& orig)
{
    std::vector<BabelSource *>::iterator it;

    for (it = sources.begin(); it != sources.end(); ++it)
    {// through all sources search for same prefix and routerid
        if(((*it)->getPrefix() == p) && ((*it)->getOriginator() == orig))
        {// found same
            return (*it);
        }
    }

    return NULL;
}


BabelSource *BabelSourceTable::addSource(BabelSource *source)
{
    ASSERT(source != NULL);

    BabelSource *intable = findSource(source->getPrefix(), source->getOriginator());

    if(intable != NULL)
    {// source already in table
        return intable;
    }

    sources.push_back(source);

    return source;
}

void BabelSourceTable::removeSource(BabelSource *source)
{
    std::vector<BabelSource *>::iterator it;

    for (it = sources.begin(); it != sources.end();)
    {// through all sources
        if((*it) == source)
        {// found same
            delete (*it);
            it = sources.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void BabelSourceTable::removeSources()
{
    std::vector<BabelSource *>::iterator it;

    for (it = sources.begin(); it != sources.end(); ++it)
    {// through all routes
        delete (*it);
    }
    sources.clear();
}
}
