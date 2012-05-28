//
// Copyright (C) 2011 Martin Danko
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "DscpTtlCorrector.h"
#include "IPDatagram.h"


Define_Module(DscpTtlCorrector);

/*
 * initialize(): 
 * Metoda pre inicializaciu modulu.
 */

void DscpTtlCorrector::initialize()
{
  // ziska ukazovatel na modul traffic generatoru
  tg = TrafGenAccess().get(); 
}
/*
 * initialize(): 
 * Spracuje spravu, ktora pride cez vstupnu branu modulu.
 * Rozbali paket a nastavi DSCP a TTL podla definicie toku 
 * @param msg - prichadzajuca sprava 
 */
 
void DscpTtlCorrector::handleMessage(cMessage *msg)
{
  // zistenie indexu vstupnej brany a nazvu
  cGate* gate = msg->getArrivalGate();
	std::string name = gate->getBaseName();
	int index = gate->getIndex();
	
	if (name == "toNetworkLayerIn")
	{ // paket prichadzajuci z generatora -> musi byt upraveny
    cMessage *copy = msg->dup(); 

    cPacket *test = (cPacket*)(copy);
		std::string type = test->getClassName();
	  if (type == "IPDatagram")
	  { // zmena s tyka len IP paketov
      IPDatagram *ipData = dynamic_cast<IPDatagram*> (copy);
      IPAddress srcAdd =  ipData->getSrcAddress();
      IPAddress destAdd =  ipData->getDestAddress();
      
      unsigned char tos = 0;
      short ttl = 32;
      TCPSegment *tcppacket;
      UDPPacket *udppacket;
      int srcPort;
      int destPort;
	     
      switch (ipData->getTransportProtocol())
      { // rozbalenie IP paketu a zistenie DSCP a TTL podla definicie toku
        case IP_PROT_UDP: 
          udppacket = dynamic_cast<UDPPacket *> (ipData->decapsulate());
          srcPort = udppacket->getSourcePort();
          destPort = udppacket->getDestinationPort();
          tos = tg->getDscpByFlowInfo(srcAdd, destAdd, IP_PROT_UDP, srcPort, destPort);
          ttl = tg->getTtlByFlowInfo(srcAdd, destAdd, IP_PROT_UDP, srcPort, destPort);
          tg->updateSentStats(udppacket->decapsulate());
          delete udppacket;
          break;
        case IP_PROT_TCP: 
          tcppacket = dynamic_cast<TCPSegment *> (ipData->decapsulate());
          srcPort = tcppacket->getSrcPort(); 
          destPort = tcppacket->getDestPort();
          tos = tg->getDscpByFlowInfo(srcAdd, destAdd, IP_PROT_TCP, srcPort, destPort);
          ttl = tg->getTtlByFlowInfo(srcAdd, destAdd, IP_PROT_TCP, srcPort, destPort);
          cPacket *cpkt;
          uint32 endSeqNo;
          while ((cpkt=tcppacket->removeFirstPayloadMessage(endSeqNo))!=NULL)
          {
            tg->updateSentStats(cpkt);
          }
          delete tcppacket;
          break;
        default:
          break;
      }
      // zmena pozadovanych hodnot
      ipData = dynamic_cast<IPDatagram*> (msg);
      ipData->setTimeToLive(ttl);
      ipData->setDiffServCodePoint(tos);
    }
	  // vysle zmeneny paket na vystupnu vranu 
    this->send(msg, "ifOut", index);
    delete copy;
  }
  else
  { // paket prichadzajuci zo siete -> posiela sa bez zmeny
    this->send(msg, "toNetworkLayerOut", index);
  }
}


