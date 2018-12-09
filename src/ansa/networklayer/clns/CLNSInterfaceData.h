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

#ifndef ANSA_NETWORKLAYER_CLNS_CLNSINTERFACEDATA_H_
#define ANSA_NETWORKLAYER_CLNS_CLNSINTERFACEDATA_H_

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/clns/ClnsAddress.h"

namespace inet {

class INET_API CLNSInterfaceData : public InterfaceProtocolData
{
  private:
    int circuitID;
    int metric;
  public:
    enum {F_METRIC};

    CLNSInterfaceData();
    virtual ~CLNSInterfaceData();

  protected:
    void changed1(int fieldId) { changed(interfaceClnsConfigChangedSignal, fieldId); }


  public:

    int getMetric() const { return metric; }
    virtual void setMetric(int m) { metric = m; changed1(F_METRIC); }

    int getCircuitId() const
    {
      return circuitID;
    }

    void setCircuitId(int circuitId)
    {
      circuitID = circuitId;
    }
};

} /* namespace inet */

#endif /* ANSA_NETWORKLAYER_CLNS_CLNSINTERFACEDATA_H_ */
