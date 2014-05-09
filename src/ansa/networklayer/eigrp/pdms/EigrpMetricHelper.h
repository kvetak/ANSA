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

#include "EigrpInterfaceTable.h"
#include "EigrpMessage_m.h"

class EigrpMetricHelper
{
  private:
    // Constants for metric computation
    const uint32_t DELAY_PICO;
    const uint32_t BANDWIDTH;
    const uint32_t CLASSIC_SCALE;
    const uint32_t WIDE_SCALE;

    unsigned int getMin(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p1 : p2; }
    unsigned int getMax(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p2 : p1; }

  public:
    // Constants for unreachable route
    static const uint64_t DELAY_INF = 0xFFFFFFFFFFFF;       // 2^48
    static const uint64_t BANDWIDTH_INF = 0xFFFFFFFFFFFF;   // 2^48
    static const uint64_t METRIC_INF = 0xFFFFFFFFFFFFFF;    // 2^56

    EigrpMetricHelper();
    virtual ~EigrpMetricHelper();

    EigrpWideMetricPar getParam(EigrpInterface *eigrpIface);
    EigrpWideMetricPar adjustParam(const EigrpWideMetricPar& ifParam, const EigrpWideMetricPar& neighParam);
    uint64_t computeClassicMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues);
    uint64_t computeWideMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues);
    bool compareParameters(const EigrpWideMetricPar& par1, const EigrpWideMetricPar& par2, EigrpKValues& kValues);
    bool isParamMaximal(const EigrpWideMetricPar& par) { return par.delay == DELAY_INF; }
};

#endif /* EIGRPMETRICHELPER_H_ */
