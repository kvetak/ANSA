// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
/**
* @file RIPngInterface.h
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIPng Interface
* @detail Represents RIPng interface
*/

#ifndef RIPNGINTERFACE_H_
#define RIPNGINTERFACE_H_

#include "UDPSocket.h"

class RIPngProcess;

namespace RIPng
{

struct Socket;
struct GlobalSocket;

class Interface
{
  public:
    Interface(int intId, RIPngProcess *process);
    virtual ~Interface();

  protected:
    int id;                               ///< id of the interface as is in interfaceTable
    RIPngProcess *pProcess;               ///< pointer to the RIPng process to which interface belongs
    Socket *pOutputSocket;                ///< pointer to the socket structure, which is used to send messages out of this interface
    bool bPassive;                        ///< if interface is passive
    bool bSplitHorizon;                   ///< if split horizon is enabled on this interface
    bool bPoisonReverse;                  ///< if poison reverse is enabled on this interface

    bool bDefaultInformation;             ///< CISCO default-information
    bool bDefaultRouteOnly;               ///< if true = default-information only, false = default-information originate
    int bDefaultRouteMetric;              ///< metric of the default route

    int iMetricOffset;                    ///< CISCO metric-offset

  public:
    /**
     * Start sending default route out of the interface with metric 1. See Cisco RIPng documentation.
     */
    void setDefaultInformationOriginate();

    /**
     * Start sending default route ONLY out of the interface with metric 1. See Cisco RIPng documentation.
     */
    void setDefaultInformationOnly();

    /**
     * Stop sending default route out of the interface.
     */
    void noDefaultInforamtion();
    bool defaultInformation() { return bDefaultInformation; }
    bool defaultRouteOnly() { return bDefaultRouteOnly; }

    void setDefaultRouteMetric(int defaultRouteMetric) { if (defaultRouteMetric >=1 && defaultRouteMetric <= 15) bDefaultRouteMetric = defaultRouteMetric; }
    int getDefaultRouteMetric() { return bDefaultRouteMetric; }

    /**
     * Sets metric-offset. See Cisco RIPng documentation.
     */
    bool setMetricOffset(int metricOffset) { if (metricOffset >= 1 && metricOffset <=16) { iMetricOffset = metricOffset; return true; } return false; }
    int getMetricOffset() { return iMetricOffset; }

    void enablePassive()   { bPassive = true; }
    void disablePassive() { bPassive = false; }
    bool isPassive() { return bPassive; }

    void enableSplitHorizon()  { bSplitHorizon = true; }
    void disableSplitHorizon()  { bSplitHorizon = false; }
    bool isSplitHorizon()   { return bSplitHorizon; }

    void enablePoisonReverse()  { bPoisonReverse = true; }
    void disablePoisonReverse()  { bPoisonReverse = false; }
    bool isPoisonReverse()   { return bPoisonReverse; }

    void setOutputSocket(Socket *outputSocket) { pOutputSocket = outputSocket; }
    Socket *getOutputSocket() { return pOutputSocket; }

    int getId() { return id; }
    RIPngProcess *getProcess() { return pProcess; }
};

struct Socket
{
    UDPSocket socket;
    int port;
    std::vector<Interface *> RIPngInterfaces;  ///< Interfaces using that socket

    int removeInterface(Interface *interface)
    {
        int numInterfaces = RIPngInterfaces.size();
        for (int i = 0; i < numInterfaces; ++i)
        {
            if (interface == RIPngInterfaces[i])
            {
                RIPngInterfaces.erase(RIPngInterfaces.begin()+i);
                return numInterfaces - 1;
            }
        }

        return numInterfaces;
    }
};

struct GlobalSocket
{
    UDPSocket socket;                          ///< Socket for "send" messages with global unicast address as a source
    std::vector<RIPngProcess *> processes;     ///< RIPng processes using that socket

    int removeProcess(RIPngProcess *process)
    {
        int numProcesses = processes.size();
        for (int i = 0; i < numProcesses; ++i)
        {
            if (process == processes[i])
            {
                processes.erase(processes.begin()+i);
                return numProcesses - 1;
            }
        }

        return numProcesses;
    }
};

} /* namespace RIPng */

#endif /* RIPNGINTERFACE_H_ */
