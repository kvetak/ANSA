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


#include <omnetpp.h>
#include "AnsaQosSystem.h"
#include "RoutingTableAccess.h"


Define_Module(AnsaQosSystem);

/*
 * ~AnsaQosSystem(): 
 * Destruktor modulu, ktory odstrani vsetky pakety v podfrontach.
 */
 
AnsaQosSystem::~AnsaQosSystem()
{
  for (AnsaSubQueues::iterator sbqIt = subQueues.begin(); sbqIt != subQueues.end(); ++sbqIt)
  {
    sbqIt->clearSubQueue();
  }
  
  delete[] subqlenVec;
  delete[] subdropVec;
}

/*
 * initialize(): 
 * Metoda pre inicializaciu modulu.
 * @param stage - aktualna sekvencia volania
 */

void AnsaQosSystem::initialize(int stage)
{
  if(stage == 0)
  {
    PassiveQueueBase::initialize();

    outGate = gate("out");
    
    WATCH(QueueInfo);
    WATCH_VECTOR(subQueues);
  }
  else if (stage == 4)
  {
    // nacitanie konfiguracie zo XML
    loadConfigFromXML();
  }

}

/*
 * loadConfigFromXML(): 
 * Metoda pre nacitanie konfiguracie modulu zo XML.
 */
 
void AnsaQosSystem::loadConfigFromXML()
{
  IRoutingTable* rt = RoutingTableAccess().get();
  
  const char *fileName = par("configFile");
  cXMLElement* document = ev.getXMLDocument(fileName);
  if (document == NULL)
  { 
    error("Cannot read AS configuration from file %s", fileName);
  }
  /* Ziskanie elementu konfiguracie daneho smerovaca */
  std::string routerXPath("Router[@id='");
  std::string routerId = rt->getRouterId().str();
  routerXPath += routerId;
  routerXPath += "']";
 
  cXMLElement* router = document->getElementByPath(routerXPath.c_str());
  if (router == NULL)
  {
    error("No configuration for Router ID: %s", routerId.c_str());
  }

  /* Ziskanie elementu konfiguracie daneho rozhrania */
  std::string itfName = this->getParentModule()->getName();
  itfName += (char)('0' + this->getParentModule()->getIndex());
  std::string intXPath(".//Interface[@name='");
  intXPath += itfName;
  intXPath += "']";

  cXMLElement* qInt = router->getElementByPath(intXPath.c_str());
  if (qInt == NULL)
  {
    error("No configuration for Interface: %s", itfName.c_str());
  }
  
  /* Ziskanie elementu konfiguracie vystupnej fronty */
  cXMLElement* clsfrEl =  NULL;
  cXMLElement* qs = qInt->getFirstChildWithTag("OutputQueue");
  if (qs == NULL)
  { // ziadna konfiguracia -> defaultne FIFO o dlzke 75
    QueueInfo.setFIFO(75);
    subQueues.push_back(ANSAQOS::SubQueue(ANSAQOS::Q_FIFO, ANSAQOS::S_FIFO, QueueInfo.getHoldCapacity(), *clsfrEl));
    return;
  }
 
  /* Nacitanie konfiguracie podla typu fronty */
  std::string qType = qs->getAttribute("type");
  if(qType == "WFQ")
  {
    QueueInfo.setWFQ();
    cXMLElementList ifDetails = qs->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ++ifElemIt) 
    {
      std::string nodeName = (*ifElemIt)->getTagName();
      if (nodeName == "HoldQueueLength")
        QueueInfo.setHoldCapacity(atoi((*ifElemIt)->getNodeValue()));
      if (nodeName == "CDT")
        QueueInfo.setCdt(atoi((*ifElemIt)->getNodeValue()));
      if (nodeName == "DynamicQueues")
        QueueInfo.setMaxQueues(atoi((*ifElemIt)->getNodeValue()));
    }
  }
  else if(qType == "FIFO")
  {
    int len = 75;
    cXMLElementList ifDetails = qs->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ++ifElemIt) 
    {
      std::string nodeName = (*ifElemIt)->getTagName();
      if (nodeName == "HoldQueueLength")
        len = atoi((*ifElemIt)->getNodeValue());
    }
    QueueInfo.setFIFO(len);
    subQueues.push_back(ANSAQOS::SubQueue(ANSAQOS::Q_FIFO, ANSAQOS::S_FIFO, QueueInfo.getHoldCapacity(), *clsfrEl));
  }
  else if(qType == "PQ")
  {
    QueueInfo.setPQ();

    cXMLElement* pql = qs->getFirstChildWithTag("PriorityQueueList");
    if (pql == NULL)
    {
      error("No priority-list specified on interface: %s", itfName.c_str());
    }
    
    QueueInfo.setListId(pql->getNodeValue());
    std::string pqlXPath(".//PriorityQueueList[@id='");
    pqlXPath += QueueInfo.getListId();
    pqlXPath += "']";

    

    pql = router->getElementByPath(pqlXPath.c_str());
    if (pql == NULL)
    {
      error("Priority-list %s is not defined", QueueInfo.getListId().c_str());
    }
    initSubQueueVectors(ANSAQOS::Q_PQ);
    setPQSubQueue ("HighQueue", pql, 20);
    setPQSubQueue ("MediumQueue", pql, 40);
    setPQSubQueue ("NormalQueue", pql, 60);
    setPQSubQueue ("LowQueue", pql, 80);       
  }
  else if(qType == "CQ")
  {
    QueueInfo.setCQ();

    cXMLElement* pql = qs->getFirstChildWithTag("CustomQueueList");
    if (pql == NULL)
    {
      error("No custom-list specified on interface: %s", itfName.c_str());
    }
    
    QueueInfo.setListId(pql->getNodeValue());
    std::string pqlXPath(".//CustomQueueList[@id='");
    pqlXPath += QueueInfo.getListId();
    pqlXPath += "']";

    pql = router->getElementByPath(pqlXPath.c_str());
    if (pql == NULL)
    {
      error("Custom-list %s is not defined", QueueInfo.getListId().c_str());
    }
    
    initSubQueueVectors(ANSAQOS::Q_CQ);
    for(int i = 0; i < 16; ++i)
      setCQSubQueue (i+1, pql);
  }
   
}

/*
 * setPQSubQueue(): 
 * Metoda nacita konfiguraciu a vytvori podfrontu PQ 
 * @param id - identifikator podfronty
 * @param pql - XML blok s konfiguracou vystupnej fronty 
 */

void AnsaQosSystem::setPQSubQueue (std::string id, cXMLElement* pql, int len)
{
    cXMLElement* clsfr = NULL;
    std::string elemDQ;
    
    cXMLElementList pqlDetails = pql->getChildren();
    for (cXMLElementList::iterator pqlElemIt = pqlDetails.begin(); pqlElemIt != pqlDetails.end(); ++pqlElemIt) 
    {
      std::string nodeName = (*pqlElemIt)->getTagName();
      if (nodeName == "DefaultQueue")
         elemDQ = (*pqlElemIt)->getNodeValue();
      if (nodeName == id)
      {
        cXMLElementList qDetails = (*pqlElemIt)->getChildren();
        for (cXMLElementList::iterator qElemIt = qDetails.begin(); qElemIt != qDetails.end(); ++qElemIt)
        {
          std::string nn = (*qElemIt)->getTagName();
          if (nn == "Lenght")
            len = atoi((*qElemIt)->getNodeValue());
          if (nn == "Classifier")
            clsfr = *qElemIt;
        } 
      }
    }
    
    subQueues.push_back(ANSAQOS::SubQueue(ANSAQOS::Q_PQ, ANSAQOS::S_FIFO, len, *clsfr));
    int d = subQueues.size();
    subQueues.back().setQueueId(d);
    subQueues.back().setVectorPointer(&subqlenVec[d-1], &subdropVec[d-1]);
    QueueInfo.addHoldCapacity(len);

    if (elemDQ.length() > 0 && elemDQ[2] == id[2])
       QueueInfo.setDefaultQueueId(subQueues.size());
  
    
}

/*
 * setCQSubQueue(): 
 * Metoda nacita konfiguraciu a vytvori podfrontu CQ 
 * @param id - identifikator podfronty
 * @param pql - XML blok s konfiguracou vystupnej fronty 
 */

void AnsaQosSystem::setCQSubQueue (int id, cXMLElement* pql)
{
    cXMLElement* clsfr = NULL;
    int dqVal;
    int len = 20;
    int bc = 1500;
    
    cXMLElementList pqlDetails = pql->getChildren();
    for (cXMLElementList::iterator pqlElemIt = pqlDetails.begin(); pqlElemIt != pqlDetails.end(); ++pqlElemIt) 
    {
      std::string nodeName = (*pqlElemIt)->getTagName();
      if (nodeName == "DefaultQueue")
         dqVal = atoi((*pqlElemIt)->getNodeValue());
      if (nodeName == "CustomQueue" && atoi((*pqlElemIt)->getAttribute("id")) == id)
      {
        cXMLElementList qDetails = (*pqlElemIt)->getChildren();
        for (cXMLElementList::iterator qElemIt = qDetails.begin(); qElemIt != qDetails.end(); ++qElemIt)
        {
          std::string nn = (*qElemIt)->getTagName();
          if (nn == "Lenght")
            len = atoi((*qElemIt)->getNodeValue());
          if (nn == "ByteCount")
            bc = atoi((*qElemIt)->getNodeValue());
          if (nn == "Classifier")
            clsfr = *qElemIt;
        } 
      }
    }
    
    subQueues.push_back(ANSAQOS::SubQueue(ANSAQOS::Q_CQ, ANSAQOS::S_FIFO, len, *clsfr));
    subQueues.back().setQueueId(id);
    subQueues.back().setWeight(bc);
    subQueues.back().setActualBytes(bc);
    subQueues.back().setVectorPointer(&subqlenVec[id-1], &subdropVec[id-1]);
    QueueInfo.addHoldCapacity(len);

    if (dqVal == id)
       QueueInfo.setDefaultQueueId(id);
   
}

/*
 * getLowestSn(): 
 * Funkcia najde najnizsie sekvencne cislo vramci vsetkych podfront. 
 * @return - Vrati hodnotu najnizsieho sekvencneho cisla
 */

int AnsaQosSystem::getLowestSn()
{
    int sn = -1;
    
    for (AnsaSubQueues::iterator sbqIt = subQueues.begin(); sbqIt != subQueues.end(); ++sbqIt)
    {
      if(sn == -1 && !sbqIt->isQueueEmpty())
      {
        sn = sbqIt->getLowestSn();
      }
      else if(!sbqIt->isQueueEmpty() && sbqIt->getLowestSn() < sn )
      {
        sn = sbqIt->getLowestSn();
      }
    }  
    return sn;
}



/*
 * initSubQueueVectors(): 
 * Funkcia vytvori vektory pre PQ a CQ
 * @param sn - typ fronty
 */

void AnsaQosSystem::initSubQueueVectors(ANSAQOS::QueueType type)
{
 std::stringstream forStr;
 switch(type)
  {
    case ANSAQOS::Q_PQ :
      subqlenVec = new cOutVector[4];
      subqlenVec[0].setName("High Length");
      subqlenVec[1].setName("Medium Length");
      subqlenVec[2].setName("Normal Length");
      subqlenVec[3].setName("Low Length");
      subdropVec = new cOutVector[4];
      subdropVec[0].setName("High Drops");
      subdropVec[1].setName("Medium Drops");
      subdropVec[2].setName("Normal Drops");
      subdropVec[3].setName("Low Drops");  
      break;
    case ANSAQOS::Q_CQ :
      subqlenVec = new cOutVector[16];
      subdropVec = new cOutVector[16]; 
      for(int i = 0; i < 16; ++i)
      {
        forStr.str(std::string());
        forStr << "Q" << i+1 << " Length";
        subqlenVec[i].setName(forStr.str().c_str());
        forStr.str(std::string());
        forStr << "Q" << i+1 << " Drops";
        subdropVec[i].setName(forStr.str().c_str());
      }
      break;
    default :
      break;
  }  
}

/*
 * getSubQueueBySn(): 
 * Funkcia najde frontu obsahujucu paket z danym sekvencnym cislom
 * @param sn - sekvencne cislo paketu, ktory ma byt najdeny
 * @return - Vrati iterator na frontu s paketom daneho sekvencneho cisla 
 */

AnsaSubQueues::iterator AnsaQosSystem::getSubQueueBySn(int sn)
{
    for (AnsaSubQueues::iterator sbqIt = subQueues.begin(); sbqIt != subQueues.end(); ++sbqIt)
    {
      if(sn == sbqIt->getLowestSn())
      {
        return sbqIt;
      }
    }
    return subQueues.end();
}

/*
 * substractSnInAllQueues(): 
 * Metoda, ktora odcita hodnotu zo sekvencneho cisla paketov vsetkych podfront. 
 * @param sn	- hodnota, ktora ma byt zo sekvencnych cisel odcitana 
 */

void AnsaQosSystem::substractSnInAllQueues(int sn)
{
    for (AnsaSubQueues::iterator sbqIt = subQueues.begin(); sbqIt != subQueues.end(); ++sbqIt)
    {
      sbqIt->substractSnFromAll(sn);
    }
}

/*
 * enqueue(): 
 * Funkcia prevadza operaciu vlozenia spravy do fronty. 
 * @param msg	- sprava, ktora ma byt vlozena do fronty
 * @return - Vrati true pokial bol paket zahodeny
 */

bool AnsaQosSystem::enqueue(cMessage *msg)
{   
    if(QueueInfo.getCurrentHold() < QueueInfo.getHoldCapacity())
    { // je miesto vo fronte
      for(AnsaSubQueues::iterator sbqIt = subQueues.begin(); sbqIt != subQueues.end(); ++sbqIt)
      { // prechadza vsetky podfronty
        short res = sbqIt->enqueue(msg,true);
        if(res == ANSAQOS::QUEUED)
        {
          QueueInfo.addCurrentHold();
          return false; // paket bol vlozeny do fronty
        }
        else if(res == ANSAQOS::NOTQUEUED)
        {
          QueueInfo.addDroped();
          delete msg;
          return true;  // paket zahodeny lebo sa nevojde do podfronty
        }
      }
      if(QueueInfo.getQueueType() == ANSAQOS::Q_WFQ)
      { // vo WFQ neexistuje podfronta pre dany tok
        if(QueueInfo.getNumQueues() < QueueInfo.getMaxQueues())
        { // je mozne vytvorit podfrontu a vlozit do nej paket 
          subQueues.push_back(ANSAQOS::SubQueue(msg, QueueInfo.getCdt()));
          QueueInfo.addNumQueues();
          QueueInfo.addCurrentHold();
          return false;
        }
      }
      else if(QueueInfo.getQueueType() == ANSAQOS::Q_PQ || QueueInfo.getQueueType() == ANSAQOS::Q_CQ)
      { // u PQ a CQ moznost vlozenia paketu do defaultnej fronty 
        if (subQueues[QueueInfo.getDefaultQueueId()-1].enqueue(msg,false) == ANSAQOS::QUEUED)
        {
          QueueInfo.addCurrentHold();
          return false;
        }
      }  
    }

    QueueInfo.addDroped();
    delete msg;  
    return true; //paket bol zahodeny
}

/*
 * dequeue(): 
 * Funkcia prevadza operaciu vybrania spravy z fronty. 
 * @return - Vrati ukazovatel na vybrany paket 
 */

cMessage *AnsaQosSystem::dequeue()
{
    if(QueueInfo.getCurrentHold() <= 0)
    {
        return NULL; // fronta je prazdna
    }
    
    cMessage *msg; cPacket *pkt;
    int sn,id;  
    AnsaSubQueues::iterator queueIt;
    
    switch (QueueInfo.getQueueType())
    {
      /* FIFO fronta ma len jednu "podfrontu" z ktorej sa vyberie prvy paket */
      case ANSAQOS::Q_FIFO : 
        msg = subQueues.begin()->dequeue();
        break;
      /* Vo WFQ fronte sa vyberie paket s najmensim sekvencnym cislom.
         V pripade, ze podfronta sa vyprazdni, tak je odstranena.
         Aby nedoslo k preteceniu hodnoty sekvencneho cisla, tak je po kazdom 
         vybrani paketu jeho sekvencne cislo odcitane od ostatnych */
      case ANSAQOS::Q_WFQ :
        sn = getLowestSn();
        queueIt = getSubQueueBySn(sn);
                
        msg = queueIt->dequeue();
        if(queueIt->isQueueEmpty())
        {
          subQueues.erase(queueIt);
          QueueInfo.remNumQueues();
        }
        substractSnInAllQueues(getLowestSn());
        break;
      /* PQ fronta obsahuje 4 fronty, ktore prechadza podla priority
         prvy paket, na ktory narazi je vybrany */
      case ANSAQOS::Q_PQ :
          for (AnsaSubQueues::iterator queueIt = subQueues.begin(); queueIt != subQueues.end(); ++queueIt)
          {
            if(!queueIt->isQueueEmpty())
            {
              msg = queueIt->dequeue();
              break;
            }
          }
        break;
      /* CQ obsahuje 16 front, ktore su prechadzane round robin. 
         Pakety su z fronty vyberane az dovtedy, kym obsahuje dostatocny 
         pocet bajtov alebo je fronta prazdna*/
      case ANSAQOS::Q_CQ :
          while (subQueues[QueueInfo.getCurrentCQQueueId()].isQueueEmpty())
          { // najdenie prvej neprazdnej fronty
            subQueues[QueueInfo.getCurrentCQQueueId()].setActualBytes();
            QueueInfo.addCurrentCQQueueId();
          }
          
          id = QueueInfo.getCurrentCQQueueId();
          msg = subQueues[id].dequeue(); // vybranie paketu z fronty
          pkt = dynamic_cast<cPacket *> (msg->dup());
          subQueues[id].remActualBytes(pkt->getByteLength()); // odstanenie bajtov z fronty
          delete pkt;
          
          if(subQueues[id].getActualBytes() <= 0)
          { // nedostatocny pocet bajtov.. prechod na dalsiu frontu a reset bajtov
            subQueues[id].setActualBytes();
            QueueInfo.addCurrentCQQueueId();  
          }
        break; 
      default :
        msg = NULL;
        break;
    }

    QueueInfo.remCurrentHold();
    return msg;
}

/*
 * sendOut(): 
 * Metoda odosle spravu cez vystupnu branu
 * @param msg - sprava, ktora ma byt odoslana
 */

void AnsaQosSystem::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

/*
 * SubQueue(): 
 * Konstruktor vytvorenia podfronty pre WFQ na zaklade spravy
 * @param msg - sprava, ktora definuje tok pre klasifikator podfronty
 * @param cdt - dlzka vytvaranej fronty 
 */

ANSAQOS::SubQueue::SubQueue(cMessage *msg, int cdt)
{
  qlenVec = dropVec = NULL;
  lastSn = 0;
  maxLength = cdt;
  qt = Q_WFQ;
  sbqt = S_FIFO;
  droped = 0;
  this->setWeight(msg);
  clsfr = new WFQClassifier(msg);
  this->enqueue(msg,true);
}

/*
 * SubQueue(): 
 * Konstruktor pre vytvorenie podfronty na zaklade vstupnych parametrov
 * @param t1 - typ fronty, ktorej je podfronta sucastou
 * @param t2 - typ podfronty
 * @param len - dlzka vytvaranej fronty   
 * @param clsfrConfig - XML blok s konfiguraciu klasifikatora podfronty
 */

ANSAQOS::SubQueue::SubQueue(QueueType t1, SubQueueType t2, int len, cXMLElement& clsfrConfig)
{
  qlenVec = dropVec = NULL;
  lastSn = 0;
  maxLength = len;
  qt = t1;
  sbqt = t2;
  droped = 0;
  
  switch (qt) // na zaklade typu fronty sa vytvori klasifikator podfronty
  {
    case Q_FIFO : 
      clsfr = new MatchAnyClassifier();
      break;
    case Q_PQ :
    case Q_CQ :
      if (&clsfrConfig != NULL)
      {
        std::string cType = clsfrConfig.getAttribute("type");
        if(cType == "DSCP")
          clsfr = new DSCPClassifier(clsfrConfig);
        else if (cType == "PREC")
          clsfr = new PRECClassifier(clsfrConfig);
        else if (cType == "ACL")
          clsfr = new ACLClassifier(clsfrConfig);   
      }
      else
        clsfr = new MatchNoneClassifier();
      break;
    default :
      break;
  }
  
}

/*
 * clearSubQueue(): 
 * Metoda vymaze vsetky pakety z podfronty. 
 */

void ANSAQOS::SubQueue::clearSubQueue()
{
    if(!isQueueEmpty())
    {
        for (std::vector<SNPacket>::iterator it = snPackets.begin(); it!=snPackets.end(); ++it)
        {
            delete it->getMsg();
            it->setMsg(NULL);
        }
    }    
}

/*
 * enqueue(): 
 * Funkcia prevadza operaciu vlozenia spravy do subfronty. 
 * @param msg	- sprava, ktora ma byt vlozena do subfronty
 * @param classify - parameter urcujuci ci sa ma vykonat klasifikacia 
 * @return - Vrati vysledok operacie 
 */

short ANSAQOS::SubQueue::enqueue(cMessage *msg, bool classify)
{
  int sn = 0;
  
  if(classify && !clsfr->classifyPacket(msg))
    return NOMATCH; // paket nebol do fronty klasifikovany
  
  if(this->getQueueLength() >= maxLength)
  {
    this->addDroped();
    this->updateVectorDrop();
    return NOTQUEUED; // fronta je plna
  }  
  
  switch (qt)
  {
    case Q_WFQ : 
      sn = this->calculateSn(msg); // vypocet SN v pripade WFQ fronty
      setLastSn(sn);
      break;
    default :
      break;  
  }
  
  SNPacket newSNPacket(msg, sn);
  snPackets.push_back(newSNPacket); // vlozenie paketu do podfronty
  
  this->updateVectorLenght();
  return QUEUED;  // paket vlozeny do fronty
}

/*
 * dequeue(): 
 * Funkcia vyberie sparavy z podfronty
 * @return - Vrati vybranu spravu  
 */

cMessage *ANSAQOS::SubQueue::dequeue()
{
    cMessage *msg = (snPackets.front()).getMsg();
    snPackets.erase(snPackets.begin());
    
    this->updateVectorLenght();
    return msg;
}

/*
 * calculateSn(): 
 * Funkcia vypocita sekvencne cislo pre spravu na zaklade 
 * hodnoty jej IP precedence a velkosti. 
 * @param msg	- sprava pre ktoru je vypocitavane sekvencne cislo
 * @return - Vrati sekvencne cislo danej spravy  
 */

int ANSAQOS::SubQueue::calculateSn(cMessage *msg)
{
  int sn;
  
  cPacket *pkt = dynamic_cast<cPacket *> (msg->dup());
  int len = pkt->getByteLength();
  
  short ipPrec = 0;
  if (dynamic_cast<IPDatagram *>(pkt))
  {
    IPDatagram *datagram = (IPDatagram *)pkt;
    ipPrec = (short) datagram->getDiffServCodePoint()/32;
  }
  
  delete pkt;
  
  sn = this->getLastSn() + ((32384 / (ipPrec + 1)) * len);

  return sn;
}

/*
 * substractSnFromAll(): 
 * Metoda, ktora odpocita zo vsetkych sekvencych cisel paketov danej fronty 
 * pozadovanu hodnotu. 
 * @param subNum	- hodnota, ktora bude zo sekvencnych cisel odcitana
 */

void ANSAQOS::SubQueue::substractSnFromAll(int subNum)
{
    if(!isQueueEmpty())
    {
        for (std::vector<SNPacket>::iterator it = snPackets.begin(); it!=snPackets.end(); ++it)
        {
            it->substractSn(subNum);
        }
        lastSn -= subNum;
    }
}

/*
 * setWeight(): 
 * Metoda, ktora vypocita na zaklade hodnoty IP precedence paketu vahu
 * danej podfronty. 
 * @param msg	- sprava, podla ktorej sa urci hodnota vyahy
 */

void ANSAQOS::SubQueue::setWeight(cMessage *msg)
{
  Flow flowID;
  flowID.parseFromMsg(msg);
  weight = 32384 / (flowID.ipPrec +1); // vypocet vahy pre podfrontu
}

/*
 * setVectorName(): 
 * Metoda, ktora inicializuje nazvy vektorov pre statistiku fronty
 * @param type	- typ fronty
 */

void ANSAQOS::QueueConfig::setVectorName(ANSAQOS::QueueType type)
{
  switch(type)
  {
    case ANSAQOS::Q_FIFO :
      qlenVec->setName("FIFO Length");
      dropVec->setName("FIFO Drops");
      break;
    case ANSAQOS::Q_WFQ : 
      qlenVec->setName("WFQ Length");
      dropVec->setName("WFQ Drops");
      break;
    case ANSAQOS::Q_PQ : 
      qlenVec->setName("PQ Length");
      dropVec->setName("PQ Drops");
      break;
    case ANSAQOS::Q_CQ : 
      qlenVec->setName("CQ Length");
      dropVec->setName("CQ Drops");
      break;
    default :
      break;
  } 
}

/*
 * Pretazenie operatoru "<<" pre vypis stavu fronty do grafickeho rozhrania 
 */

std::ostream& operator<< (std::ostream& ostr, ANSAQOS::QueueConfig config)
{ 
  switch (config.getQueueType())
  {
    case ANSAQOS::Q_FIFO :
      ostr  << "Strategy: fifo;  Queue: ";
      ostr  << config.getCurrentHold() << "/";
      ostr  << config.getDroped() << "/";
      ostr  << config.getHoldCapacity() << " (size/dropped/max)";
      break;
    case ANSAQOS::Q_WFQ : 
      ostr  << "Strategy: weighted fair;  Queue: ";
      ostr  << config.getCurrentHold() << "/";
      ostr  << config.getHoldCapacity() << "/";
      ostr  << config.getCdt() << "/";
      ostr  << config.getDroped() << " (size/max total/threshold/drops); Conversations: ";
      ostr  << config.getNumQueues() << "/";
      ostr  << config.getMaxActQueues() << "/";
      ostr  << config.getMaxQueues() << " (active/max active/max total)";
      break;
    case ANSAQOS::Q_PQ : 
      ostr  << "Strategy: priority-list " << config.getListId();
      switch (config.getDefaultQueueId())
      {
        case 1 :
          ostr  << " (Default queue: High)";
          break;
        case 2 :
          ostr  << " (Default queue: Medium)";
          break;
        case 3 :
          ostr  << " (Default queue: Normal)";
          break;
        case 4 :
          ostr  << " (Default queue: Low)";
          break;
        default:
          ostr  << " (Default queue: Error)";
      }
      break;
    case ANSAQOS::Q_CQ : 
      ostr  << "Strategy: custom-list " << config.getListId();
      ostr  << " (Default queue: " << config.getDefaultQueueId() << ")";
      break;
    default :
      ostr  << "Error";
  }    
  
  return ostr;
}

