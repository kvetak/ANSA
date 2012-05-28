//
// Copyright 2011 Martin Danko
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __ANSA_TRAFGEN_H
#define __ANSA_TRAFGEN_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "ITrafGenApplication.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"
#include "UDPSocket.h"
#include "IPControlInfo.h"
#include "UDPControlInfo_m.h"
#include "TrafGenPacket_m.h"
#include "TCPSegment.h"
#include "UDPPacket.h"

namespace TG {

/* Trieda zaznamenanie casu a velkosti paketu */
class PktRec
{
  private:
    double time;
    int length;
  
  public:
    PktRec(double t, int l) {time = t; length = l;}
    double getTime() {return time;}
    int getLength() {return length;}
    
}; 

/* Trieda pre ukladanie statistik o prijatych paketoch toku */ 
class RcvFlowRecord
{
private:
    std::string id;          // identifikator toku
    double startTime;        // cas prichodu prveho paketu
    double endTime;          // cas prichodu posledneho paketu
    long  totalSentPkts;     // celkovy pocet zaslanych paketov
    long  totalRcvPkts;      // celkovy pocet prijatych paketov
    double minDelay;         // minimalne onesekorenie
    double maxDelay;         // maximalne oneskorenie
    double totalDelay;       // celkove oneskorenie
    double totalJitter;      // celkovy jitter
    long totalBytes;         // celkovy pocet prijatych bajtov
    std::vector<PktRec> actStat; // vektor s paketmi za poslednu sekundu
    

public:
    RcvFlowRecord();

    std::string getId() {return id;}
    void setId(std::string n_id) {id = n_id;}
    double getStartTime() {return startTime;}
    void setStartTime(double n_startTime) {startTime = n_startTime;}
    double getEndTime() {return endTime;}
    void setEndTime(double n_endTime) {endTime = n_endTime;}

    void addTotalSentPkts() {++totalSentPkts;}
    void addTotalRcvPkts() {++totalRcvPkts;}
    void setMinDelay(double del) {minDelay = del;}
    void setMaxDelay(double del) {maxDelay = del;}
    void addTotalDelay(double del) {totalDelay += del;}
    void addTotalJitter(double jit) {totalJitter += jit;}
    void addTotalBytes(long bytes) {totalBytes += bytes;}
    long getTotalSentPkts() {return totalSentPkts;}
    long getTotalRcvPkts() {return totalRcvPkts;}
    double getMinDelay() {return minDelay;}
    double getMaxDelay() {return maxDelay;}
    double getTotalDelay() {return totalDelay;}
    double getTotalJitter() {return totalJitter;}
    long getTotalBytes() {return totalBytes;}
    void updateActVec(double t, int l) {actStat.push_back(PktRec(t,l));}
    int getActualPacketrate() {actualizeVector(); return actStat.size();} 
    double getActBitrate();
    void actualizeVector();
    

};

typedef std::vector<RcvFlowRecord> RcvStats;

/* Trieda pre ukladanie statistik o prijatych paketoch toku */ 
class SntFlowRecord {

private:
    std::string id;          // identifikator toku
    double startTime;        // cas odoslania prveho paketu
    long  totalSentPkts;     // celkovy pocet zaslanych paketov
    long totalBytes;         // celkovy pocet zaslanych bajtov

public:
    SntFlowRecord() {totalSentPkts = 0; totalBytes = 0;}
    
    std::string getId() {return id;}
    void setId(std::string n_id) {id = n_id;}
    double getStartTime() {return startTime;}
    void setStartTime(double n_startTime) {startTime = n_startTime;}
    

    void addTotalSentPkts() {++totalSentPkts;}
    void addTotalBytes(long bytes) {totalBytes += bytes;}
    long getTotalSentPkts() {return totalSentPkts;}
    long getTotalBytes() {return totalBytes;}

};

typedef std::vector<SntFlowRecord> SntStats;  

/* Trieda reprezentujuca zaznam o jedenom toku */ 
class FlowRecord
{
private:
    std::string id;           // identifikator toku
    double startTime;         // cas sa ma tok zacat generovat 
    double duration;          // doba generovania toku
    IPvXAddress srcIP;        // zdrojova IP adresa toku
    IPvXAddress dstIP;        // cielova IP adresa toku
    unsigned char tos;        // ToS (type of service)
    short ttl;                // TTL (time to live)
    short protocol;           // trasportny protokol
    int srcPort;              // zdrojovy port
    int dstPort;              // cielovy port
    
    std::string appName;                // nazov aplikacie toku
    ITrafGenApplication *pApplication;  // ukazovatel na objekt aplikacie 
    
    bool generating;          // urcuje ci dany modul tok generuje
    bool analyzing;           // urcuje ci dany modul tok analyzuje
    
    RcvStats::iterator rcvModStatsIt;   // iterator na statistiky prijemcu
    
    TCPSocket *socket;        // TCP socket v pripade pouzitia TCP trasportu 
    

public:
    FlowRecord();
    
    std::string getId() {return id;}
    void setId(std::string n_id) {id = n_id;}
    double getStartTime() {return startTime;}
    void setStartTime(double n_startTime) {startTime = n_startTime;}
    double getDuration() {return duration;}
    void setDuration(double n_duration) {duration = n_duration;}
    IPvXAddress getSrcIP() {return srcIP;}
    void setSrcIP(IPvXAddress n_srcIP) {srcIP = n_srcIP;}
    IPvXAddress getDstIP() {return dstIP;}
    void setDstIP(IPvXAddress n_dstIP) {dstIP = n_dstIP;}
    unsigned char getTos() {return tos;}
    void setTos(unsigned char n_tos) {tos = n_tos;}
    short getTtl() {return ttl;}
    void setTtl(short n_ttl) {ttl = n_ttl;}
    short getProtocol() {return protocol;}
    void setProtocol(short n_protocol) {protocol = n_protocol;}
    int getSrcPort() {return srcPort;}
    void setSrcPort(int n_srcPort) {srcPort = n_srcPort;}
    int getDstPort() {return dstPort;}
    void setDstPort(int n_dstPort) {dstPort = n_dstPort;}
    
    ITrafGenApplication * getPApplication() {return pApplication;}
    std::string getAppName() {return appName;}
    bool setApplication(const cXMLElement& appConfig);
    
    bool isGenerating() {return generating; }
    void setGenerating(bool n_gen) {generating = n_gen;}
    bool isAnalyzing() {return analyzing; }
    void setAnalyzing(bool n_ana) {analyzing = n_ana;}
    
    RcvStats::iterator getRcvModStatsIt() {return rcvModStatsIt;}
    void setRcvModStatsIt(RcvStats::iterator n_it) {rcvModStatsIt = n_it;}
    
    void tcpConnect(cGate *toTcp);
    void tcpClose() {socket->close();}
    void sendTcpPacket(cMessage *msg) {socket->send(msg);}
    bool isSocketCreated() {return socket != NULL;}
    bool isTcpConnected() { return (this->isSocketCreated() && socket->getState() != TCPSocket::CONNECTED); }
    int getSocketConId() {return socket->getConnectionId();}
    
};

typedef std::vector<FlowRecord> Flows;

/*
 * Pretazenie operatoru "<<" pre vypis statistiky odoslanych paketov 
 * do grafickeho rozhrania 
 */
inline std::ostream& operator<< (std::ostream& ostr, SntFlowRecord& rec)
{
    double time = simTime().dbl() - rec.getStartTime();
    ostr << "Flow " << rec.getId() << " : " ;
    if(rec.getTotalSentPkts() > 0)
    {
      ostr << "Pkts sent: " << rec.getTotalSentPkts() << ", ";
      ostr << "Bytes sent: " << rec.getTotalBytes() << ", ";
      ostr << "Avg bitrate: " << (rec.getTotalBytes() * 8)/(time * 1000) << " Kbit/s, ";
      ostr << "Avg pkts: " << rec.getTotalSentPkts()/time << " pkt/s";
    }
    else
      ostr << "no sent data yet";
       
    return ostr;
}

/*
 * Pretazenie operatoru "<<" pre vypis statistiky prijatych paketov 
 * do grafickeho rozhrania 
 */
inline std::ostream& operator<< (std::ostream& ostr, RcvFlowRecord& rec)
{
    double time = simTime().dbl() - rec.getStartTime();
    ostr << "Flow " << rec.getId() << " : " ;
    if(rec.getTotalRcvPkts() > 0)
    {
      ostr << "Pkts rcv: " << rec.getTotalRcvPkts() << ", ";
      ostr << "Bytes rcv: " << rec.getTotalBytes() << ", ";
      ostr << "Avg bitrate: " << (rec.getTotalBytes() * 8)/(time * 1000) << " Kbit/s, ";
      ostr << "Avg pkts: " << rec.getTotalRcvPkts()/time << " pkt/s, ";
      ostr << "Min delay: " << rec.getMinDelay() << " s, ";
      ostr << "Max delay: " << rec.getMaxDelay() << " s, ";
      ostr << "Avg delay: " << rec.getTotalDelay() / rec.getTotalRcvPkts() << " s, ";
      ostr << "Avg jitter: " << rec.getTotalJitter() / rec.getTotalRcvPkts() << " s, ";
    }
    else
      ostr << "no received data yet";
     
    return ostr;
}

}

/* Trieda reprezentujuca modul generatora datovych tokov */ 
class INET_API TrafGen : public cSimpleModule
{
  protected:
  
    TG::Flows flows;                       // definovane toky
    TG::SntStats sentStatistics;           // statistiky o generovanych tokoch 
    TG::RcvStats receivedStatistics;       // statistiky o analyzovanych tokoch
    std::vector<int> tcpListenPort;        // zoznam tcp nasluchajucich portov
    std::vector<int> udpListenPort;        // zoznam udp nasluchajucich portov
    
    std::map<std::string, cOutVector*> actBitrateVec; //vektory pre zaznam aktualej bitovej rychlosti
    std::map<std::string, cOutVector*> actPacketrateVec; //vektory pre zaznam aktualej paketovej rychlosti  
  
    int numSent;           // pocet zaslanych paketov
    int numReceived;       // pocet prijatych paketov
    int counter;           // citac
    
    
    cPacket *createPacket(TG::Flows::iterator flowIt);
    bool sendPacket(TG::Flows::iterator flowIt);
    void processPacket(cMessage *msg);
    
    bool LoadFlowsFromXML(const char * filename);
    void parseFlow(const cXMLElement& flowConfig);
    bool isValidFlow(TG::FlowRecord &flw);
    unsigned char readDscp(std::string dscpString);
    void detectRoles();
    void installRcvStatIts(); 
    void initTimers();
    void bindSockets();
    void initStats();
    bool isLocalTcpOrig(int id);
    bool isListening(short prot, int port);
    void updateActVectorStats();
    
    TG::Flows::iterator getFlowById(std::string s_id);
    TG::SntStats::iterator getSntFlowStatById(std::string s_id);
    TG::RcvStats::iterator getRcvFlowStatById(std::string r_id);
    
  public:
    ~TrafGen();
    unsigned char getDscpByFlowInfo(IPAddress &srcAdd,IPAddress &destAdd, short prot, int srcPort, int destPort);
    short getTtlByFlowInfo(IPAddress &srcAdd,IPAddress &destAdd, short prot, int srcPort, int destPort);
    void updateSentStats(cPacket * payload);


  protected:
    virtual int numInitStages() const {return 6;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


