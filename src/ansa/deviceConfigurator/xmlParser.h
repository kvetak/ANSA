//
// Marek Cerny, 2MSK
// FIT VUT 2011
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

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <omnetpp.h>

using namespace std;

/*
 * Set of static methods that can be used by multiple modules for unified
 * access to various pars of the configuration XML file.
 *
 * The idea is to provide tools that will take care about tag parsing while
 * the calling module can focus on extracting actual aplication-specific data.
 */
class xmlParser {
   public:
      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, const char *configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);
      static cXMLElement * GetOspfProcess6(cXMLElement *process, cXMLElement *device);
      static cXMLElement * GetIPv6Address(cXMLElement *addr, cXMLElement *iface);
      static cXMLElement * GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      static cXMLElement * GetIsisRouting(cXMLElement * device);
      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);
};

#endif /* XMLPARSER_H_ */
