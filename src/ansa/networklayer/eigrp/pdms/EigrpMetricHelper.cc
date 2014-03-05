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

#include <EigrpMetricHelper.h>

EigrpMetricHelper::EigrpMetricHelper()
{
    // TODO Auto-generated constructor stub
}

EigrpMetricHelper::~EigrpMetricHelper()
{
    // TODO Auto-generated destructor stub
}

/**
 * Sets parameters from interface for metric computation.
 */
EigrpMetricPar EigrpMetricHelper::getParam(EigrpInterface *eigrpIface, InterfaceEntry *iface)
{
    EigrpMetricPar newMetricPar;
    uint32_t bw, dly;

    bw = eigrpIface->getBandwidth();
    dly = eigrpIface->getDelay();

    if (bw != 0)
        newMetricPar.bandwidth = 10000000 / bw * 256;

    newMetricPar.delay = dly / 10 * 256;
    newMetricPar.load = eigrpIface->getLoad();
    newMetricPar.reliability = eigrpIface->getReliability();
    newMetricPar.hopCount = 0;
    newMetricPar.mtu = iface->getMTU();

    return newMetricPar;
}

/**
 * Adjust parameters of metric by interface parameters.
 */
EigrpMetricPar EigrpMetricHelper::adjustParam(const EigrpMetricPar& ifParam, const EigrpMetricPar& neighParam)
{
    EigrpMetricPar newMetricPar;

    // Bandwidth and delay are precomputed
    newMetricPar.bandwidth = getMax(ifParam.bandwidth, neighParam.bandwidth);
    newMetricPar.load = getMax(ifParam.load, neighParam.delay);
    newMetricPar.reliability = getMin(ifParam.reliability, neighParam.reliability);
    newMetricPar.mtu = getMin(ifParam.mtu, neighParam.mtu);
    newMetricPar.hopCount++;

    if (neighParam.delay == UINT32_MAX)
        newMetricPar.delay = UINT32_MAX;   // Infinite metric
    else
        newMetricPar.delay = ifParam.delay + neighParam.delay;

    return newMetricPar;
}

/**
 * Computes classic metric.
 * TODO: nahradit hodnoty konstantami, viz RFC (i pro případní rozšíření o Wild metric)
 */
uint32_t EigrpMetricHelper::computeMetric(const EigrpMetricPar& par, const EigrpKValues& kValues)
{
    uint32_t metric;

    if (par.delay == UINT32_MAX)
        return UINT32_MAX;  // Infinite metric

    metric = kValues.K1*par.bandwidth + kValues.K2*par.bandwidth / (256 - par.load) + kValues.K3 * par.delay;
    if (kValues.K5 != 0)
        metric = metric * kValues.K5 / (par.reliability + kValues.K4);

    return metric;
}

bool EigrpMetricHelper::compareParamters(const EigrpMetricPar& par1, const EigrpMetricPar& par2, EigrpKValues& kValues)
{
    if (kValues.K1 && par1.bandwidth != par2.bandwidth)
        return false;
    if (kValues.K2 && par1.load != par2.load)
        return false;
    if (kValues.K3 && par1.delay != par2.delay)
        return false;
    if (kValues.K5 && par1.reliability != par2.reliability)
        return false;

    return true;
}
