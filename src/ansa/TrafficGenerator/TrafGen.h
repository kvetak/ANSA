//
// Copyright 2010 Martin Danko
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

namespace TG {


class RcvFlowRecord {

private:
    std::string id;
    double startTime;
    double endTime;
    long  totalSentPkts;
    long  totalRcvPkts;
    double minDelay;
    double maxDelay;
    double totalDelay;
    double totalJitter;
    long totalBytes;

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

};

typedef std::vector<RcvFlowRecord> RcvStats;


class SntFlowRecord {

private:
    std::string id;
    double startTime;
    long  totalSentPkts;
    long totalBytes;

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

class FlowRecord {

private:
    std::string id;
    double startTime;
    double duration;
    IPvXAddress srcIP;
    IPvXAddress dstIP;
    unsigned char tos;
    short ttl;
    short protocol;
    int srcPort;
    int dstPort;
    
    std::string appName;
    ITrafGenApplication *pApplication;
    
    bool generating;
    bool analyzing;
    
    RcvStats::iterator rcvModStatsIt;
    

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
};

typedef std::vector<FlowRecord> Flows;

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

/**

 */
class INET_API TrafGen : public cSimpleModule
{
  protected:
  
    TG::Flows flows;
    TG::SntStats sentStatistics;
    TG::RcvStats receivedStatistics;
  
    int numSent;
    int numReceived;
    int counter;
    
    
    cPacket *createPacket(TG::Flows::iterator flowIt);
    void sendPacket(TG::Flows::iterator flowIt);
    void processPacket(cPacket *msg);
    
    bool LoadFlowsFromXML(const char * filename);
    void parseFlow(const cXMLElement& flowConfig);
    bool isValidFlow(TG::FlowRecord &flw);
    unsigned char readDscp(std::string dscpString);
    void detectRoles();
    void installRcvStatIts(); 
    void initTimers();
    void bindSockets();
    void initStats();
    
    TG::Flows::iterator getFlowById(std::string s_id);
    TG::SntStats::iterator getSntFlowStatById(std::string s_id);
    TG::RcvStats::iterator getRcvFlowStatById(std::string r_id);


  protected:
    virtual int numInitStages() const {return 6;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


