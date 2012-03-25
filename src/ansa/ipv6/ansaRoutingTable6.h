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

#ifndef ANSAROUTINGTABLE6_H_
#define ANSAROUTINGTABLE6_H_

#include "RoutingTable6.h"

class AnsaIPv6Route : public IPv6Route {
   protected:
      bool active;
      std::string _interfaceName;

   public:
      AnsaIPv6Route(IPv6Address destPrefix, int length, RouteSrc src) : IPv6Route(destPrefix, length, src){};

      std::string printRoute() const;
      bool isActive() const  { return active; }
      void setActive(bool a) { active = a;    }

      void setInterfaceName(const char *interfaceName)   { _interfaceName = interfaceName;   }
      const char * getInterfaceName() const              { return _interfaceName.c_str();    }
};


class AnsaRoutingTable6 : public RoutingTable6 {
   protected:
      std::vector<AnsaIPv6Route*> *routes;

   public:
      AnsaRoutingTable6();
      const IPv6Route *doLongestPrefixMatch(const IPv6Address& dest);
      void addDirectRoute(const IPv6Address& destPrefix, int prefixLength, unsigned int interfaceId);
   protected:
      virtual int numInitStages() const  {return 5;}
      virtual void initialize(int stage);
      virtual void receiveChangeNotification(int category, const cPolymorphic *details);
      static bool routeLessThan(const IPv6Route *a, const IPv6Route *b);
      virtual void addRoute(IPv6Route *route);
};

#endif /* ANSAROUTINGTABLE6_H_ */
