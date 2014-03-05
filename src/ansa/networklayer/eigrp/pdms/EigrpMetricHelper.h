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

#ifndef EIGRPMETRICHELPER_H_
#define EIGRPMETRICHELPER_H_

#include "InterfaceEntry.h"

#include "EigrpRoute.h"
#include "EigrpInterfaceTable.h"
#include "EigrpMessage_m.h"

class EigrpMetricHelper
{
  private:
    unsigned int getMin(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p1 : p2; }
    unsigned int getMax(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p2 : p1; }

  public:
    EigrpMetricHelper();
    virtual ~EigrpMetricHelper();

    EigrpMetricPar getParam(EigrpInterface *eigrpIface, InterfaceEntry *iface);
    EigrpMetricPar adjustParam(const EigrpMetricPar& ifParam, const EigrpMetricPar& neighParam);
    uint32_t computeMetric(const EigrpMetricPar& par, const EigrpKValues& kValues);
    bool compareParamters(const EigrpMetricPar& par1, const EigrpMetricPar& par2, EigrpKValues& kValues);
};

#endif /* EIGRPMETRICHELPER_H_ */
