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

#include <LISPSiteRecord.h>

LISPSiteRecord::LISPSiteRecord() {

}

LISPSiteRecord::~LISPSiteRecord() {

}

LISPServerEntry& LISPSiteRecord::getServerEntry() {
    return ServerEntry;
}

void LISPSiteRecord::setServerEntry(const LISPServerEntry& serverEntry) {
    ServerEntry = serverEntry;
}

std::string LISPSiteRecord::info() const {
    std::stringstream os;
    os << "ETR " << ServerEntry.info() << endl;
    os << LISPMapStorageBase::info();
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPSiteRecord& sr) {
    return os << sr.info();
}

bool LISPSiteRecord::operator ==(const LISPSiteRecord& other) const {
    return ServerEntry == other.ServerEntry &&
           MappingStorage == other.MappingStorage;
}
