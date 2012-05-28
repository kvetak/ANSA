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


#ifndef __ANSA_QOS_CLASSIFIERS_H
#define __ANSA_QOS_CLASSIFIERS_H

#include "IPDatagram.h"
#include "TCPSegment.h"
#include "UDPPacket.h"
#include "AclContainerAccess.h"

namespace ANSAQOS {

/*
 * Trieda reprezentuje tok definovany 6-ticou:
 * - zdrojova IP adresa
 * - cielova IP adresa
 * - transportny protokol
 * - IP precedence
 * - zdrojovy port
 * - cielovy port      
 */
  
class Flow {

public:
    IPAddress srcAddress;
    IPAddress destAddress;
    int transportProtocol;
    short ipPrec;
    int srcPort;
    int destPort;
    
    Flow() {}
    Flow(cMessage *msg) {this->parseFromMsg(msg);}

    void parseFromMsg(cMessage *msg);

    bool operator==(const Flow& flow) const;

};

/*
 * Trieda definuje rozhranie klasifikatora     
 */

class Classifier
{
public:
    virtual bool classifyPacket(cMessage *msg) = 0; // rozhodne o klasifikacii
    virtual std::string info() = 0; // vrati string pre graficke rozhranie
}; 

/*
 * Trieda klasifikatora pre WFQ frontu na zaklade toku     
 */

class WFQClassifier : public Classifier
{
private: 
    Flow flowID;
    std::string infoString;
        
public:
    WFQClassifier(cMessage *msg);   
    virtual bool classifyPacket(cMessage *msg);
    virtual std::string info() {return infoString;}
};

/*
 * Trieda klasifikatora na zaklade ACL     
 */

class ACLClassifier : public Classifier
{
private: 
    std::vector<std::string> acls;
    AclContainer* aclAccess;
    std::string infoString;     
public:
    ACLClassifier(cXMLElement& clsfrConfig);   
    virtual bool classifyPacket(cMessage *msg);
    virtual std::string info() {return infoString;}
};

/*
 * Trieda klasifikatora na zaklade DSCP     
 */

class DSCPClassifier : public Classifier
{
private: 
    std::vector<unsigned char> values;
    std::string infoString; 
        
public:
    DSCPClassifier(cXMLElement& clsfrConfig);
    unsigned char readDscp(std::string dscpString);   
    virtual bool classifyPacket(cMessage *msg);
    virtual std::string info() {return infoString;}
};

/*
 * Trieda klasifikatora na zaklade IP precedence     
 */

class PRECClassifier : public Classifier
{
private: 
    std::vector<short> values;
    std::string infoString; 
        
public:
    PRECClassifier(cXMLElement& clsfrConfig);   
    virtual bool classifyPacket(cMessage *msg);
    virtual std::string info() {return infoString;}
};

/*
 * Trieda klasifikatora, ktory klasifikuje kazdy paket     
 */

class MatchAnyClassifier : public Classifier
{
        
public:
    virtual bool classifyPacket(cMessage *msg) {return true;}
    virtual std::string info() {return "Match any";}
};

/*
 * Trieda klasifikatora, ktory neklasifikuje ziadny paket     
 */

class MatchNoneClassifier : public Classifier
{
        
public:
    virtual bool classifyPacket(cMessage *msg) {return false;}
    virtual std::string info() {return "Match none";}
};

}

#endif

