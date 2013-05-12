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
* @file RIPngInterface.cc
* @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief
* @detail
*/

#ifndef RIPNGROUTING_H_
#define RIPNGROUTING_H_

#include <omnetpp.h>

#include "UDPSocket.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"

#include "RIPngProcess.h"
#include "RIPngInterface.h"

#include "RIPngMessage_m.h"

/**
 *  Represents routing protocol RIPng. RIPng uses UDP and communicates
 *  using UDPSocket.
 */
class RIPngRouting : public cSimpleModule, protected INotifiable
{
  public:
    RIPngRouting();
    virtual ~RIPngRouting();

  protected:
    IInterfaceTable* ift;    ///< Provides access to the interface table.
    NotificationBoard* nb;   ///< Provides access to the notification board

    const char  *deviceId;   ///< Id of the device which contains this routing process.
    std::string hostName;    ///< Device name from the network topology.
    std::string routerText;  ///< Text with hostName for ev printing.

    int          connNetworkMetric;
    int          infinityMetric;
    simtime_t    routeTimeout;
    simtime_t    routeGarbageCollectionTimeout;
    simtime_t    regularUpdateTimeout;
    IPv6Address  RIPngAddress;
    int          RIPngPort;
    unsigned int distance;

    std::vector<RIPngProcess *> processes;  ///< Running RIPng processes

    typedef std::map<int, RIPng::GlobalSocket *> GlobalSockets;
    GlobalSockets globalSockets;

    //interface, port - key
    typedef std::pair<int, int> SocketsKey;
    typedef std::map<SocketsKey, RIPng::Socket *> Sockets;
    Sockets sockets;

  public:
    std::string getHostName() { return hostName; }

    unsigned int getDistance() { return distance; }
    int getConnNetworkMetric() { return connNetworkMetric; }
    int getInfinityMetric() { return infinityMetric; }
    simtime_t getRouteTimeout() { return routeTimeout; }
    simtime_t getRouteGarbageCollectionTimeout() { return routeGarbageCollectionTimeout; }
    simtime_t getRegularUpdateTimeout() { return regularUpdateTimeout; }
    IPv6Address getRIPngAddress() { return RIPngAddress; }
    int getRIPngPort() { return RIPngPort; }


    //-- COMMANDS
    void setPortAndAddress(const char *processName, int port, IPv6Address &address);
    void setPortAndAddress(RIPngProcess *process, int port, IPv6Address &address);
    void setDistance(const char *processName, int distance);
    void setDistance(RIPngProcess *process, int distance);
    void setPoisonReverse(const char *processName, bool poisonReverse);
    void setPoisonReverse(RIPngProcess *process, bool poisonReverse);
    void setSplitHorizon(const char *processName, bool splitHorizon);
    void setSplitHorizon(RIPngProcess *process, bool splitHorizon);
    /**
     * If you don't want to set some of the timers pass -1 as the parameter.
     */
    void setTimers(const char *processName, int update, int route, int garbage);
    void setTimers(RIPngProcess *process, int update, int route, int garbage);

    //-- INTERFACES METHODS
    /**
     * Enable RIPng with the given name on the interface. If process with the processName does not
     * exists, it creates and start new process.
     * (creates new RIPng::Interface and adds it to the process "RIPng interface table").
     * @param processName [in] process name
     * @param interfaceName [in] interfaceName
     * @return created RIPng interface
     */
    RIPng::Interface *enableRIPngOnInterface(const char *processName, const char *interfaceName);
    RIPng::Interface *enableRIPngOnInterface(RIPngProcess *process, InterfaceEntry *interface);

    /**
     * Disable RIPng with the given name on the interface.
     * @param processName [in] process name
     * @param interfaceName [in] interfaceName
     */
    void disableRIPngOnInterface(const char *processName, const char *interfaceName);
    void disableRIPngOnInterface(RIPngProcess *process, int RIPngInterfaceIndex);
    /**
     * Sets port which is used in the messages on the interface. It creates and binds
     * socket for that purposes. Used port on the interface can be changed by calling
     * this method again with the different port parameter.
     * -1 in the port parameter means no more messages will be sent on that interface
     * using that port (used by RIPng processes when disabling interfaces).
     * @param port [in] port which should be used in the messages on the interface
     *
     * @see moveInterfaceToSocket
     */
    RIPng::Interface *setOutputPortOnInterface(RIPng::Interface *interface, int port);
  private:
    /**
     * If RIPng process needs to send message it uses one of the RIPng socket from sockets.
     * Socket is binded to the interface (in parameter) address and port (in parameter) which is
     * used as a source and destination port in the sent messages.
     *
     * Messages are sent through the sockets. Correct socket is found by the output interface
     * of the message and by the port which should be used as a source and destination port.
     *
     * @see setOutputPortOnInterface
     */
    void moveInterfaceToSocket(RIPng::Interface *interface, int port);

  public:
    //-- PROCESS MANAGEMENT
    /**
     * Get RIPng process by the given name
     * @param processName [in] RIPng process name
     * @return RIPngProcess or NULL if no process for the given name have been found
     */
    RIPngProcess *getProcess(const char *processName);
    unsigned int getProcessIndex(const char *processName);

    /**
     * Creates new RIPng process with the given name
     * @param processName [in] RIPng process name
     * @return created process
     */
    RIPngProcess *addProcess(const char *processName);

    /**
     * Stops and deletes process with the given name
     * @param processName [in] RIPng process name
     */
    void          removeProcess(const char *processName);

    /**
     * RIPng process receives messages through socket which is binded to the port (parameter)
     * and joined to the multicastAddress (parameter).
     * If RIPng process change port and multicastAddress, this method must be called and the last
     * used port must be passed as oldPort. (New port and address must be set to the process - by calling
     * methods setRIPngPort() and setRIPngAddress() - and match to the port and multicastAddress parameters)
     * If process is initializing -1 is passed as oldPort.
     * All RIPng process interfaces are moved to the proper socket for sending messages @see moveInterfaceToSocket.
     */
    void          moveProcessToSocket(RIPngProcess *process, int oldPort, int port, IPv6Address &multicastAddress);

    //-- MESSAGES HANDLING
    void sendMessage(RIPngMessage *msg, IPv6Address &addr, int port, int interfaceId, bool globalSourceAddress);

  protected:
    /**
     * Forwards RIPngMessage to the processes.
     */
    void forwardRIPngMessage(RIPngMessage *msg);

    //-- TIMERS
    /**
     * Forwards timer messages to the processes.
     */
    void forwardRIPngTimer(RIPngTimer *timer);

  public:
    /**
    * Creates new timer
    * @param timerKind [in] type of the timer @see RIPngTimer
    * @param context [in] context pointer @see cMessage
    * @return Created RIPngTimer timer
    * @see cMessage
    */
    RIPngTimer *createTimer(int timerKind, void *context);

    /**
    * Creates and starts new timer
    * @param timerKind [in] type of the timer @see RIPngTimer
    * @param context [in] context pointer @see cMessage
    * @param timerLen [in] timer duration
    * @return Created RIPngTimer timer
    * @see cMessage
    */
    RIPngTimer *createAndStartTimer(int timerKind, void *context, simtime_t timerLen);

    /**
    * Starts or resets timer
    * @param timer [in] timer
    * @param timerLen [in] timer duration
    */
    void resetTimer(RIPngTimer *timer, simtime_t timerLen);

    /**
    * Cancels timer
    * @param timer [in] timer
    */
    void cancelTimer(RIPngTimer *timer);

    /**
    * Deletes timer (timer will be canceled if is running)
    * @param timer [in] timer
    */
    void deleteTimer(RIPngTimer *timer);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void updateDisplayString();
    virtual void receiveChangeNotification(int category, const cObject *details);

};

#endif /* RIPNGROUTING_H_ */
