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
 */

#include <LISPTStructs.h>

std::ostream& operator <<(std::ostream& os, const TAfiAddrs& tas) {
    for (TAfiAddrCItem it = tas.begin(); it != tas.end(); ++it)
        os << it->info() << endl;
    return os;
}

std::ostream& operator <<(std::ostream& os, const TRecs& recs) {
    for (TRecsCItem it = recs.begin(); it != recs.end(); ++it) {
        os << it->info() << endl;
    }
    return os;
}

std::string TAfiAddr::info() const {
    std::stringstream os;
    os << (afi() == 2 ? "(IPv6) ": "(IPv4) ") << address;
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const TAfiAddr& taa) {
    return os << taa.info();
}


std::string TRecord::info() const {
    std::stringstream os;
    os << EidPrefix << endl
       << "TTL: " << recordTTL << " minutes"
       << ", action: " << short(ACT) << "(" << this->getActionString() << ")";
    if (ABit)
        os << ", Authoritative";
    if (mapVersion)
        os << ", map-version: " << mapVersion << endl;
    os << "\nLocator-count: " << short(locatorCount) << ">";
    for (TLocatorCItem it = locators.begin(); it != locators.end(); ++it)
        os << it->info();
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const TRecord& trec) {
    return os << trec.info();
}

std::ostream& operator <<(std::ostream& os, const TRecords& trecs) {
    for (TRecordCItem it = trecs.begin(); it != trecs.end(); ++it )
        os << it->info() << endl << endl;
    return os;
}

std::string TLocator::info() const {
    std::stringstream os;
    os << endl << this->RLocator.getRlocAddr()
       << "\t" << short(this->RLocator.getPriority())
       << "/" << short(this->RLocator.getWeight()) << "\t";
    if (LocalLocBit)
       os << "Local";
    if (piggybackBit)
       os << ", probing";
    if (RouteRlocBit)
       os << ", Reachable";
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const TLocator& tloc) {
    return os << tloc.info();
}

const std::string TRecord::getActionString() const {
    switch (ACT) {
        case LISPCommon::DROP:
            return "Drop";
        case LISPCommon::SEND_MAP_REQUEST:
            return "Send-Map-Request";
        case LISPCommon::NATIVELY_FORWARD:
            return "Natively-Forward";
        case LISPCommon::NO_ACTION:
        default:
            return "No-Action";
    }
}
