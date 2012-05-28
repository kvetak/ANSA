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

#include "AnsaQosClassifiers.h"

/*
 * Pretazenie operatoru "==" pre porovnanie tokov 
 */

bool ANSAQOS::Flow::operator==(const Flow& flow) const
{
    if(srcAddress != flow.srcAddress)
        return false;
    if(destAddress != flow.destAddress)
        return false;
    if(transportProtocol != flow.transportProtocol)
        return false;
    if(ipPrec != flow.ipPrec)
        return false;
    if(srcPort != flow.srcPort)
        return false;
    if(destPort != flow.destPort)
        return false;
    
    return true;
}

/*
 * parseFromMsg(): 
 * Konstruktor toku, ktory nacita hodnoty zo spravy (TCP/UDP paketu)
 * @param msg	- sprava z ktora definuje parametre toku 
 */
 
void ANSAQOS::Flow::parseFromMsg(cMessage *msg)
{
  cMessage *copy = msg->dup();

  if (dynamic_cast<IPDatagram *>(copy))
  {
    IPDatagram *datagram = (IPDatagram *)copy;
        
    srcAddress = datagram->getSrcAddress();
    destAddress = datagram->getDestAddress();
    transportProtocol = datagram->getTransportProtocol();
    ipPrec = (short) datagram->getDiffServCodePoint()/32; // prve 3 bity ToS
    if(transportProtocol == IP_PROT_TCP)
    {
      TCPSegment *tcpSegm = dynamic_cast<TCPSegment *> (datagram->decapsulate());
      srcPort = tcpSegm->getSrcPort();
      destPort = tcpSegm->getDestPort();
      delete tcpSegm;
    }
    else if(transportProtocol == IP_PROT_UDP)
    {
      UDPPacket *udpDatag  = dynamic_cast<UDPPacket *> (datagram->decapsulate());
      srcPort = udpDatag->getSourcePort();
      destPort = udpDatag->getDestinationPort();
      delete udpDatag;
    }
    else
    {
      srcPort = 0;
      destPort = 0;
    }      
  }
  else
  {
    srcAddress = IPAddress::UNSPECIFIED_ADDRESS;
    destAddress = IPAddress::UNSPECIFIED_ADDRESS;
    transportProtocol = 0;
    ipPrec = 0;
    srcPort = 0;
    destPort = 0;
  }
  
  delete copy;

}

/*
 * WFQClassifier(): 
 * Konstruktor klasifikatoru WFQ fronty, ktory je vytvoreny 
 * na zaklade spravy (TCP/UDP paketu)
 * @param msg	- sprava z ktora definuje parametre toku 
 */
 
ANSAQOS::WFQClassifier::WFQClassifier(cMessage *msg)
{
  /* Nacita tok na zaklade hlaviciek spravy */
  flowID.parseFromMsg(msg);

  /* Vytvorenie stringu pre graficke rozhranie */    
  std::stringstream info;
  info << "Flow: src: ";
  info << flowID.srcAddress.str();
  info << ":";
  info << flowID.srcPort;
  info << " dest: ";
  info << flowID.destAddress.str();
  info << ":"; 
  info << flowID.destPort;
  info << " prec: ";
  info << flowID.ipPrec;
  info << " prot: ";
  info << flowID.transportProtocol; 
  infoString = info.str();  
}

/*
 * classifyPacket(): 
 * Funkcia rozhoduje o klasifikacii spravy do daneho toku
 * @param msg	- sprava, ktora ma byt klasifikovana
 * @return - Vrati true pokial je sprava klasifikovana 
 */
 
bool ANSAQOS::WFQClassifier::classifyPacket(cMessage *msg)
{
  if (flowID == Flow(msg))
      return true;
  return false;
}

/*
 * ACLClassifier(): 
 * Konstruktor ACL klasifikatoru, ktory nacita parametre zo XML. 
 * @param clsfrConfig	- XML blok s konfiguraciou klasifikatora 
 */

ANSAQOS::ACLClassifier::ACLClassifier(cXMLElement& clsfrConfig)
{
  /* Ziska pristup do ACL kontajneru smerovaca */  
  aclAccess = AclContainerAccess().get();
  
  /* Do retazca uklada informacie pre graficke rozhranie */ 
  infoString = "ACL:=";
  
  /* Prechadzanie elementov XML konfiguracie */  
  cXMLElementList aclDetails = clsfrConfig.getChildren();
  for (cXMLElementList::iterator aclElemIt = aclDetails.begin(); aclElemIt != aclDetails.end(); ++aclElemIt) 
  {
    std::string nodeName = (*aclElemIt)->getTagName();
    if (nodeName == "acl")
    {
      std::string aclname = (*aclElemIt)->getNodeValue();
      if (aclname.length() > 0 )
      {
        acls.push_back(aclname);
        infoString += " ";
        infoString += aclname;
        if(!aclAccess->existAcl(aclname))
          infoString += " (not defined)";
      }
    }
  }
   
}

/*
 * classifyPacket(): 
 * Funkcia rozhoduje o klasifikacii spravy danym ACL
 * @param msg	- sprava, ktora ma byt klasifikovana
 * @return - Vrati true pokial je sprava klasifikovana  
 */

bool ANSAQOS::ACLClassifier::classifyPacket(cMessage *msg)
{
  std::vector<std::string>::iterator it;

  for(it = acls.begin(); it != acls.end(); ++it )
  {  
    if (aclAccess->matchPacketToAcl(*it, msg))
      return true;
  }   
  return false;
}

/*
 * readDscp(): 
 * Funkcia prevedie string z DSCP hodnotou na 8 bitovu hodotu ToS
 * @param dscpString	- retazec DSCP hodnoty
 * @return - Vrati hodnotu ToS zakodovanu v DSCP retazcu  
 */

unsigned char ANSAQOS::DSCPClassifier::readDscp(std::string dscpString)
{

  if(dscpString.size() == 3)
  {
    std::string prf = dscpString.substr(0,2);
    unsigned char cls = dscpString[2] - '0';
    if((prf == "CS" || prf == "Cs" || prf == "cs") && cls > 0 && cls < 8)
      return cls*32; 
  }
  else if (dscpString.size() == 4)
  {
    std::string prf = dscpString.substr(0,2);
    unsigned char cls =  dscpString[2] - '0';
    unsigned char dp = dscpString[3] - '0';
    if((prf == "AF" || prf == "Af" || prf == "af") && cls > 0 && cls < 5 && dp > 0 && dp < 4)
      return cls*32 + dp*8;
  }
  else if(dscpString == "EF" || dscpString == "Ef" || dscpString == "ef")
    return 184;

  return atoi(dscpString.c_str());
}

/*
 * DSCPClassifier(): 
 * Konstruktor DSCP klasifikatoru, ktory nacita parametre zo XML. 
 * @param clsfrConfig	- XML blok s konfiguraciou klasifikatora 
 */

ANSAQOS::DSCPClassifier::DSCPClassifier(cXMLElement& clsfrConfig)
{
  /* Do retazca uklada informacie pre graficke rozhranie */
  infoString = "DSCP:=";

  /* Prechadzanie elementov XML konfiguracie */    
  cXMLElementList dscpDetails = clsfrConfig.getChildren();
  for (cXMLElementList::iterator dscpElemIt = dscpDetails.begin(); dscpElemIt != dscpDetails.end(); ++dscpElemIt) 
  {
    std::string nodeName = (*dscpElemIt)->getTagName();
    if (nodeName == "value")
    {
      std::string val = (*dscpElemIt)->getNodeValue();
      values.push_back(readDscp(val));
      infoString += " ";
      infoString += val;
    }
  }
   
}

/*
 * classifyPacket(): 
 * Funkcia rozhoduje o klasifikacii spravy danymi hodnotami DSCP
 * @param msg	- sprava, ktora ma byt klasifikovana
 * @return - Vrati true pokial je sprava klasifikovana  
 */

bool ANSAQOS::DSCPClassifier::classifyPacket(cMessage *msg)
{
  unsigned char tos;
  std::vector<unsigned char>::iterator it;
  cMessage *copy = msg->dup();

  if (dynamic_cast<IPDatagram *>(copy))
  {
    IPDatagram *datagram = (IPDatagram *)copy;
    tos = datagram->getDiffServCodePoint();
  }
  delete copy;
  
  for(it = values.begin(); it != values.end(); ++it )
  {  
    if (*it == tos)
      return true;
   }   
  return false;
}

/*
 * PRECClassifier(): 
 * Konstruktor IP precedence klasifikatoru, ktory nacita parametre zo XML. 
 * @param clsfrConfig	- XML blok s konfiguraciou klasifikatora 
 */

ANSAQOS::PRECClassifier::PRECClassifier(cXMLElement& clsfrConfig)
{
  /* Do retazca uklada informacie pre graficke rozhranie */
  infoString = "PREC:=";
  
  /* Prechadzanie elementov XML konfiguracie */  
  cXMLElementList dscpDetails = clsfrConfig.getChildren();
  for (cXMLElementList::iterator dscpElemIt = dscpDetails.begin(); dscpElemIt != dscpDetails.end(); ++dscpElemIt) 
  {
    std::string nodeName = (*dscpElemIt)->getTagName();
    if (nodeName == "value")
    {
      short val = (short) atoi((*dscpElemIt)->getNodeValue());
      if (val >= 0 && val <8) // IP precedence musi byt z rozsahu 0-7
      {
        values.push_back(val);
        infoString += " ";
        infoString += (*dscpElemIt)->getNodeValue();
      }
    }
  }
   
}

/*
 * classifyPacket(): 
 * Funkcia rozhoduje o klasifikacii spravy danymi hodnotami IP precedence
 * @param msg	- sprava, ktora ma byt klasifikovana
 * @return - Vrati true pokial je sprava klasifikovana  
 */

bool ANSAQOS::PRECClassifier::classifyPacket(cMessage *msg)
{
  short prec;
  std::vector<short>::iterator it;
  cMessage *copy = msg->dup();

  if (dynamic_cast<IPDatagram *>(copy))
  {
    IPDatagram *datagram = (IPDatagram *)copy;
    prec = (short) datagram->getDiffServCodePoint()/32; // prve 3bity TOS
  }
  delete copy;
  
  for(it = values.begin(); it != values.end(); ++it )
  {  
    if (*it == prec)
      return true;
   }   
  return false;
}


