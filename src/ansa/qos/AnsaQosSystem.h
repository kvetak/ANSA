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


#ifndef __ANSA_QOS_SYSTEM_H
#define __ANSA_QOS_SYSTEM_H

#include <vector>
#include <iostream>
#include <omnetpp.h>
#include "AnsaQosClassifiers.h"
#include "PassiveQueueBase.h"


namespace ANSAQOS {

/* Vycet typov fronty */
enum QueueType {
    Q_FIFO = 1,
    Q_PQ = 2,
    Q_CQ = 3,
    Q_WFQ = 4,
    Q_CBWFQ = 5,
    Q_WRED = 6
};

/* Vycet typov podfronty */  
enum SubQueueType {
    S_FIFO = 1,
    S_WFQ = 2,
    S_WRED = 3
};

/* Vycet typov vysledku operacie vlozenia do fronty */
enum EnqueueingResult {
    QUEUED = 1,
    NOMATCH = 2,
    NOTQUEUED = 3
};

/* Trieda pre uchovanie parametrov fronty */
class QueueConfig {
  private:
    QueueType qt;          // typ fronty
    std::string listId;    // ID Priority alebo Custom listu
    int numQueues;         // aktualny pocet front
    int maxActQueues;      // maximalny pocet aktivnych front
    int maxQueues;         // maximalny limit poctu front
    int currentHold;       // aktualny celkovy pocet paketov vo fronte
    int holdCapacity;      // maximalny limit poctu paketov vo fronte
    int cdt;               // Congestive Discard Threshold pre WFQ
    int droped;            // pocet zahodenych paketov
    int currentCQQueueId;  // ID prave obsluhovanej podfronty u CQ
    int defaultQueueId;    // defaultna fronta u PQ a CQ
    
    cOutVector *qlenVec;     // ukazovatel na vektor pre zaznamenanie dlzky fronty
    cOutVector *dropVec;     // ukazovatel na vektor pre zaznamenanie zahodenych paketov
    
  public:
    QueueConfig() {currentHold = 0;}
    
    void setQueueType(QueueType type) {qt = type;}
    QueueType getQueueType() {return qt;}
    void setListId(std::string id) {listId = id;}
    std::string getListId() {return listId;}
    void setNumQueues(int num) {numQueues = num;}
    int getNumQueues() {return numQueues;}
    void setMaxActQueues(int num) {maxActQueues = num;}
    int getMaxActQueues() {return maxActQueues;}
    void setMaxQueues(int num) {maxQueues = num;}
    int getMaxQueues() {return maxQueues;}
    void setCurrentHold(int num) {currentHold = num;}
    int getCurrentHold() {return currentHold;}
    void setHoldCapacity(int num) {holdCapacity = num;}
    void addHoldCapacity(int num) {holdCapacity += num;}
    int getHoldCapacity() {return holdCapacity;}
    void setCdt(int num) {cdt = num;}
    int getCdt() {return cdt;}
    void setDroped(int num) {droped = num;}
    int getDroped() {return droped;}
    void setCurrentCQQueueId(int id) {currentCQQueueId = id;}
    void addCurrentCQQueueId() {currentCQQueueId = (currentCQQueueId + 1) % 16;}
    int getCurrentCQQueueId() {return currentCQQueueId;}
    void setDefaultQueueId(int id) {defaultQueueId = id;}
    int getDefaultQueueId() {return defaultQueueId;}
    void setVectorPointer(cOutVector* l, cOutVector* d) { qlenVec = l; dropVec = d;}
    
    void addNumQueues() {++numQueues; maxActQueues = (numQueues < maxActQueues ? maxActQueues:numQueues);}
    void remNumQueues() {--numQueues;}
    void addCurrentHold() {++currentHold; qlenVec->record(currentHold);}
    void remCurrentHold() {--currentHold; qlenVec->record(currentHold);}
    void addDroped() {++droped; dropVec->record(1);}
    
    /* Inicializacia parametrov FIFO fronty */
    void setVectorName (QueueType type);
    
    void setFIFO(int hold) {qt=Q_FIFO; currentHold = droped = 0; maxQueues = numQueues = 1; 
                            holdCapacity = cdt = hold; setVectorName(Q_FIFO);}
    /* Inicializacia parametrov WFQ fronty */        
    void setWFQ()  {qt=Q_WFQ; numQueues = maxActQueues = currentHold = droped = 0; 
                    maxQueues = 256; holdCapacity = 1000; cdt = 64; setVectorName(Q_WFQ);}
    /* Inicializacia parametrov PQ fronty */                
    void setPQ() {qt=Q_PQ; currentHold = droped = holdCapacity = cdt = 0;
                  maxQueues = numQueues = 4; defaultQueueId = 3; setVectorName(Q_PQ);}
    /* Inicializacia parametrov CQ fronty */              
    void setCQ() {qt=Q_CQ; currentHold = droped = holdCapacity = cdt = 0; 
                  maxQueues = numQueues = 16; defaultQueueId = currentCQQueueId= 1; setVectorName(Q_CQ);}               
};


/* Trieda reprezentujuca paket zo sekvencnym cislom */ 
class SNPacket {

private:
    cMessage *msg;   // ukazovatel na paket
    int sn;          // sekvencne cislo paketu

public:
    SNPacket() {msg = NULL; sn = 0;}
    SNPacket(cMessage *c_msg, int c_sn) {msg = c_msg; sn = c_sn;}
    
    int getSn() {return sn;}
    void setSn(int n_sn) {sn = n_sn;}
    void substractSn(int subNum) {sn = sn - subNum;}
    
    cMessage *getMsg() {return msg;}
    void setMsg(cMessage *n_msg) {msg = n_msg;}
};


/* Trieda reprezentujuca podfrontu */
class SubQueue {

private:
    Classifier *clsfr;       // klasifikator pre podfrontu
    QueueType qt;            // typ fronty, ktorej je podfronta clenom
    SubQueueType sbqt;       // typ podfronty
    int queueId;             // ID fronty
    int maxLength;           // maximalna dlzka podfronty
    int lastSn;              // sekvencne cislo posledneho paketu fronty
    int droped;              // pocet zahodenych paketov v podfronte
    int weight;              // vaha podfronty
    int actualBytes;         // aktualny volny pocet bajtov pre CQ alg.
    
    cOutVector *qlenVec;     // vektor pre zaznamenanie dlzky fronty
    cOutVector *dropVec;     // vektor pre zaznamenanie zahodenych paketov
    
    std::vector<SNPacket> snPackets;  // ulozene pakety v podfronte
    

public:
  
    SubQueue() {lastSn = 0; qlenVec = dropVec = NULL;}
    SubQueue(cMessage *msg, int cdt);
    SubQueue(QueueType t1, SubQueueType t2, int len, cXMLElement& clsfrConfig);
    
    void clearSubQueue();
    
    void setQueueType(QueueType type) {qt = type;}
    QueueType getQueueType() {return qt;}
    void setSubQueueType(SubQueueType type) {sbqt = type;}
    SubQueueType getSubQueueType() {return sbqt;}
    void setQueueId(int id) {queueId = id;}
    int getQueueId() {return queueId;}
    void setMaxLength(int len) {maxLength = len;}
    int getMaxLength() {return maxLength;}
    void setLastSn(int n_sn) {lastSn = n_sn;}
    int getLastSn() {return lastSn;}
    void addDroped() {++droped; }
    int getDroped() {return droped;}
    void setWeight(cMessage *msg);
    void setWeight(int w) {weight = w;}
    int getWeight() {return weight;}
    void setActualBytes() {actualBytes = weight;}
    void setActualBytes(int b) {actualBytes = b;}
    void addActualBytes(int b) {actualBytes += b;}
    void remActualBytes(int b) {actualBytes -= b;}
    int getActualBytes() {return actualBytes;}
    std::string getClassifierInfo() {return clsfr->info();}
    void setVectorPointer(cOutVector* l, cOutVector* d) { qlenVec = l; dropVec = d;}
    void updateVectorLenght() {if(qlenVec) qlenVec->record(snPackets.size());}
    void updateVectorDrop() {if(dropVec) dropVec->record(1);}

    int getQueueLength() {return snPackets.size();}
    bool isQueueEmpty() {return getQueueLength()>0 ? false : true ;}
    void substractSnFromAll(int subNum);
    int getLowestSn() {return (snPackets.front()).getSn();}
    int calculateSn(cMessage *msg);
    
    short enqueue(cMessage *msg, bool classify);
    cMessage *dequeue();
};

/*
 * Pretazenie operatoru "<<" pre vypis stavu podfronty do grafickeho rozhrania 
 */

inline std::ostream& operator<< (std::ostream& ostr, SubQueue& sbq)
{
  switch (sbq.getQueueType())
  {
    case ANSAQOS::Q_FIFO :
      ostr  << "No subqueues in FIFO queue";
      break;
    case ANSAQOS::Q_WFQ : 
      ostr  << "(depth/weight/total drops) ";
      ostr << sbq.getQueueLength() << "/";
      ostr << sbq.getWeight() << "/";
      ostr << sbq.getDroped() << "  ";
      ostr << sbq.getClassifierInfo(); 
      break;
    case ANSAQOS::Q_PQ :
      switch (sbq.getQueueId())
      {
        case 1 :
          ostr  << "High: ";
          break;
        case 2 :
          ostr  << "Medium: ";
          break;
        case 3 :
          ostr  << "Normal: ";
          break;
        case 4 :
          ostr  << "Low: ";
          break;
        default:
          ostr  << "Error";
      }
      ostr << sbq.getQueueLength() << "/";
      ostr << sbq.getMaxLength() << "/";
      ostr << sbq.getDroped() ;
      ostr << " (size/max/drops)  Classifier: ";
      ostr << sbq.getClassifierInfo(); 
      
      break;
    case ANSAQOS::Q_CQ :
      ostr  << "Queue "<< sbq.getQueueId() <<": ";
      ostr << sbq.getQueueLength() << "/";
      ostr << sbq.getMaxLength() << "/";
      ostr << sbq.getDroped() ;
      ostr << " (size/max/drops)  ByteCount: ";
      ostr << sbq.getActualBytes() << "/";
      ostr << sbq.getWeight() ;
      ostr << " (actual/max)  Classifier: ";
      ostr << sbq.getClassifierInfo();
      break;
    default :
      ostr  << "Error";
  }
     
    return ostr;
}

}

typedef std::vector<ANSAQOS::SubQueue> AnsaSubQueues;

/* Trieda definujuca modul s frontami FIFO, WFQ, PQ, CQ */ 
class INET_API AnsaQosSystem : public PassiveQueueBase
{
  protected:
  
    AnsaSubQueues  subQueues;         // podfronty
    cGate *outGate;                   // vystupna brana modulu
    
    cOutVector qlenVec;     // vektor pre zaznamenanie dlzky fronty
    cOutVector dropVec;     // vektor pre zaznamenanie zahodenych paketov
    
    cOutVector *subqlenVec; // vektory podfront
    cOutVector *subdropVec; // vektory podfront
    
    ANSAQOS::QueueConfig QueueInfo;   // parametre fronty

  public:
    AnsaQosSystem() {QueueInfo.setVectorPointer(&qlenVec, &dropVec);};
    ~AnsaQosSystem();
    
    void loadConfigFromXML();
    int getLowestSn(); 
    AnsaSubQueues::iterator getSubQueueBySn(int sn);
    void substractSnInAllQueues(int sn);
    void setPQSubQueue(std::string id, cXMLElement* pql, int len);
    void setCQSubQueue (int id, cXMLElement* pql);
    void initSubQueueVectors(ANSAQOS::QueueType type);


  protected:

    virtual void initialize(int stage);

    /* Redefinovane z PassiveQueueBase. */
    virtual bool enqueue(cMessage *msg);

    /* Redefinovane z PassiveQueueBase. */
    virtual cMessage *dequeue();

    /* Redefinovane z PassiveQueueBase. */
    virtual void sendOut(cMessage *msg);
    
    virtual int numInitStages() const  { return 5;}
     
};


#endif


