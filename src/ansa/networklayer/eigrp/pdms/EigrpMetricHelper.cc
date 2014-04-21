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

EigrpMetricHelper::EigrpMetricHelper() :
    DELAY_PICO(1000000), BANDWIDTH(10000000), CLASSIC_SCALE(256), WIDE_SCALE(65536)
{
}

EigrpMetricHelper::~EigrpMetricHelper()
{
}

/**
 * Sets parameters from interface for metric computation.
 */
EigrpWideMetricPar EigrpMetricHelper::getParam(EigrpInterface *eigrpIface)
{
    EigrpWideMetricPar newMetricPar;

    newMetricPar.bandwidth = eigrpIface->getBandwidth();
    newMetricPar.delay = eigrpIface->getDelay() * DELAY_PICO;
    newMetricPar.load = eigrpIface->getLoad();
    newMetricPar.reliability = eigrpIface->getReliability();
    newMetricPar.hopCount = 0;
    newMetricPar.mtu = eigrpIface->getMtu();

    return newMetricPar;
}

/**
 * Adjust parameters of metric by interface parameters.
 */
EigrpWideMetricPar EigrpMetricHelper::adjustParam(const EigrpWideMetricPar& ifParam, const EigrpWideMetricPar& neighParam)
{
    EigrpWideMetricPar newMetricPar;

    newMetricPar.load = getMax(ifParam.load, neighParam.delay);
    newMetricPar.reliability = getMin(ifParam.reliability, neighParam.reliability);
    newMetricPar.mtu = getMin(ifParam.mtu, neighParam.mtu);
    newMetricPar.hopCount++;

    if (isParamMaximal(neighParam))
    {
        newMetricPar.delay = DELAY_INF;
        newMetricPar.bandwidth = BANDWIDTH_INF;
    }
    else
    {
        newMetricPar.delay = ifParam.delay + neighParam.delay;
        newMetricPar.bandwidth = getMax(ifParam.bandwidth, neighParam.bandwidth);
    }

    return newMetricPar;
}

/**
 * Computes classic metric.
 */
uint64_t EigrpMetricHelper::computeClassicMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues)
{
    uint32_t metric;
    uint32_t classicDelay = 0, classicBw = 0;

    if (isParamMaximal(par))
        return METRIC_INF;

    // Adjust delay and bandwidth
    if (kValues.K3)
    { // Note: delay is in pico seconds and must be converted to micro seconds for classic metric
        classicDelay = par.delay / 10000000;
        classicDelay = classicDelay * CLASSIC_SCALE;
    }
    if (kValues.K1)
        classicBw = BANDWIDTH / par.bandwidth  * CLASSIC_SCALE;

    metric = kValues.K1*classicBw + kValues.K2*classicBw / (256 - par.load) + kValues.K3 * classicDelay;
    if (kValues.K5 != 0)
        metric = metric * kValues.K5 / (par.reliability + kValues.K4);

    return metric;
}

/**
 * Computes wide metric.
 */
uint64_t EigrpMetricHelper::computeWideMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues)
{
    uint64_t metric, throughput, latency;

    if (isParamMaximal(par))
        return METRIC_INF;

    // TODO: compute delay from bandwidth if bandwidth is greater than 1 Gb/s
    // TODO: include also additional parameters associated with K6

    throughput = (BANDWIDTH * WIDE_SCALE) / par.bandwidth;
    latency = (par.delay * WIDE_SCALE) / DELAY_PICO;

    metric = kValues.K1*throughput + kValues.K2*kValues.K1*throughput/(256 - par.load) + kValues.K3*latency /*+ kValues.K6*extAttr*/;
    if (kValues.K5 != 0)
        metric = metric * kValues.K5 / (par.reliability + kValues.K4);

    return metric;
}

bool EigrpMetricHelper::compareParameters(const EigrpWideMetricPar& par1, const EigrpWideMetricPar& par2, EigrpKValues& kValues)
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
