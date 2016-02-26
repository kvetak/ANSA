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
* @file BabelSourceTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
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
