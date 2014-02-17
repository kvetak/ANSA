/*
 * stpXMLparser.cpp
 *
 *  Created on: 17.5.2011
 *      Author: aranel
 */

#include "stpXMLparser.h"

stpXMLparser::stpXMLparser(Stp * _stp) {
	stp = _stp;
}

stpXMLparser::~stpXMLparser() {

}


bool stpXMLparser::parse(const char * filename, const char * switchID) {

	cXMLElement* switchConfig = ev.getXMLDocument(filename);
	if (switchConfig == NULL) {
		return false;
	}

	std::string switchXPath("Switch[@id='");
	switchXPath += switchID;
	switchXPath += "']";

	/* FIND root of current switch's configuration subtree */
	cXMLElement* switchNode = switchConfig->getElementByPath(switchXPath.c_str());
	if (switchNode == NULL) {
		return false;
		//opp_error("No configuration for Switch ID: %s", switchID);
	}


	/* FIND section Interfaces */
	cXMLElement* stpNode = switchNode->getFirstChildWithTag("STP");
	if (stpNode != NULL) {
		parseSTP(stpNode);
	}

	return true;

}

void stpXMLparser::parseSTP(cXMLElement * node) {
	std::string tmp, instance, bridgePriority, fwDelay, helloTime,
				maxAge, portPriority, linkCost,subNodeName;

	std::vector<unsigned int> portPriList, priPortList; // list of port priorities, list of ports for adressing
	std::vector<unsigned int> portCostList, costPortList;// list of link costs, list of links for adressing

	unsigned int inst=0;

	cXMLElementList instConfig = node->getChildren();
	cXMLElementList::iterator it;

	for(it = instConfig.begin(); it != instConfig.end(); it++) {
		subNodeName  = (*it)->getTagName();

		tmp.clear();
		instance.clear();
		bridgePriority.clear();
		fwDelay.clear();
		helloTime.clear();
		maxAge.clear();
		portPriority.clear();
		linkCost.clear();

		portPriList.clear();
		priPortList.clear();
		portCostList.clear();
		costPortList.clear();

		if (subNodeName == "Instance" && (*it)->getAttribute("id")) {
			instance = (*it)->getAttribute("id");

			cXMLElementList instDetails = (*it)->getChildren();
			cXMLElementList::iterator instElemIt;
			for (instElemIt = instDetails.begin(); instElemIt != instDetails.end(); instElemIt++) {
			  std::string nodeName = (*instElemIt)->getTagName();

			  if (nodeName=="BridgePriority") {
				bridgePriority = (*instElemIt)->getNodeValue();
			  }

			  if (nodeName=="PortPriority") {
				priPortList.push_back(atoi((*instElemIt)->getAttribute("id")));
				portPriList.push_back(atoi((*instElemIt)->getNodeValue()));
			  }
			  if (nodeName=="LinkCost") {
				costPortList.push_back(atoi((*instElemIt)->getAttribute("id")));
				portCostList.push_back(atoi((*instElemIt)->getNodeValue()));
			  }
			  if (nodeName=="ForwardDelay") {
				  fwDelay = (*instElemIt)->getNodeValue();
			  }
			  if (nodeName=="HelloTimer") {
				  helloTime = (*instElemIt)->getNodeValue();
			  }
			  if (nodeName=="MaxAge") {
				  maxAge = (*instElemIt)->getNodeValue();
			  }


			} // instDetail for

		} // instConfig for

		if (instance.empty() == true) {
			continue;
		}

		inst = atoi(instance.c_str());

		if (bridgePriority.empty() == false) {
			stp->setBridgePriority(inst, atoi(bridgePriority.c_str()));
		}
		if (fwDelay.empty() == false) {
			stp->setForwardDelay(inst, atoi(fwDelay.c_str()));
		}
		if (maxAge.empty() == false) {
			stp->setMaxAge(inst, atoi(maxAge.c_str()));
		}
		if (helloTime.empty() == false) {
			stp->setHelloTime(inst, atoi(helloTime.c_str()));
		}

		if (portPriList.empty() == false) {
			stp->setPortPriority(inst, priPortList, portPriList);
		}

		if (portPriList.empty() == false) {
			stp->setPortPriority(inst, costPortList, portCostList);
		}


	}



}

