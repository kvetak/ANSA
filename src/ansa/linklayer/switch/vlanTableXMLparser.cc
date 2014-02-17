/*
 * vlanTableXMLparser.cpp
 *
 *  Created on: 16.12.2010
 *      Author: aranel
 */

#include "vlanTableXMLparser.h"

VLANTableXMLparser::VLANTableXMLparser(VLANTable * _table) {
	table = _table;
	return;
}

VLANTableXMLparser::~VLANTableXMLparser() {

}


bool VLANTableXMLparser::parse(const char * filename, const char * switchID) {

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
	    	EV << "Warning: Switch " << switchID << ": Has no config, running in default." << std::endl;
	        return false;
	    	//opp_error("No configuration for Switch ID: %s", switchID);
		}

	    /* CHECK FOR CLEAR CONFIG TAG */
	    cXMLElement* clear = switchNode->getFirstChildWithTag("ClearConfig");
		if (clear == NULL) {
			table->initDefault();
		}

	    /* FIND section Interfaces */
	    cXMLElement* IntNode = switchNode->getFirstChildWithTag("Interfaces");
	    if (IntNode != NULL) {
	    	parseInterfaces(IntNode);
	    }

	    /* FIND section VLANs */
	    cXMLElement* staticNode = switchNode->getFirstChildWithTag("VLANs");
	    if (staticNode != NULL) {
	    	parseVLANs(staticNode);
	    }

	    return true;

}


void VLANTableXMLparser::parseInterfaces(cXMLElement * node) {
	std::string tmp, port, name, vlan, subNodeName;

	cXMLElementList intConfig = node->getChildren();
	cXMLElementList::iterator it;
	for(it = intConfig.begin(); it != intConfig.end(); it++) {
		subNodeName  = (*it)->getTagName();
		if (subNodeName == "Interface" && (*it)->getAttribute("id")) {
			port = (*it)->getAttribute("id");

			cXMLElementList ifDetails = (*it)->getChildren();
			cXMLElementList::iterator ifElemIt;
			for (ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
			{
			  std::string nodeName = (*ifElemIt)->getTagName();

			  if (nodeName=="VLAN")
			  {
				vlan = (*ifElemIt)->getNodeValue();
			  }

			  if (nodeName=="Name")
			  {
				name = (*ifElemIt)->getNodeValue();
			  }

			}
		}

		if (port.empty() == false && vlan.empty() == false) {
			table->addPortVID(atoi(port.c_str()), atoi(vlan.c_str()));
		}


	}


}


void VLANTableXMLparser::parseVLANs(cXMLElement * node) {
	std::string tmp, vid, name, subNodeName;
	std::vector<int> tagged;
	std::vector<int> untagged;
	std::vector<int> nountagged;

		cXMLElementList intConfig = node->getChildren();
		cXMLElementList::iterator it;
		for(it = intConfig.begin(); it != intConfig.end(); it++) {
			subNodeName  = (*it)->getTagName();

			tagged.clear();
			untagged.clear();
			nountagged.clear();


			if (subNodeName == "VLAN" && (*it)->getAttribute("id")) {
				vid = (*it)->getAttribute("id");

				cXMLElementList ifDetails = (*it)->getChildren();
				cXMLElementList::iterator ifElemIt;
				for (ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
				{
				  std::string nodeName = (*ifElemIt)->getTagName();

				  if (nodeName == "Tagged")
				  {
					//tagged.push_back(atoi((*ifElemIt)->getNodeValue()));
					  parseList(tagged, (*ifElemIt)->getNodeValue());
				  }

				  if (nodeName == "Untagged")
				  {
					  parseList(untagged, (*ifElemIt)->getNodeValue());
				  }

				  if (nodeName == "Nountagged")
				  {
					  parseList(nountagged, (*ifElemIt)->getNodeValue());
				  }

				  if(nodeName == "Name") {
					  name = (*ifElemIt)->getNodeValue();
				  }
				}
			}

			if (vid.empty() == false) {
				int VID = atoi(vid.c_str());

				table->extendTable(VID);

				/* removing by "no untagged" command */
				for (unsigned int i = 0; i < nountagged.size(); i++) {
					table->delPort(nountagged.at(i), VID);
				}

				/* adding by "tagged" command */
				table->addTagged(VID, tagged);

				/* adding by "untagged" command */
				table->addUntagged(VID, untagged);

				table->setVLANName(VID, name);

				table->regVLAN(VID);
			}


		}

}

void VLANTableXMLparser::parseList(std::vector<int>& out, const char * in) {
	std::string tmp, xtmp;
	tmp.assign(in);
	size_t begin, end;

	begin = 0;
	end = tmp.find_first_of(",");


	if (end == std::string::npos) {
		out.push_back(atoi(tmp.c_str()));
		return;
	}

	EV << "Parsing: '" << tmp << "'\n";

	while (true) {


		if (end == std::string::npos) {
			xtmp = tmp.substr(begin, end);
		} else {
			xtmp = tmp.substr(begin, end-begin);
		}

		EV << begin << ":" << end << " " << xtmp  << "\n";

		out.push_back(atoi(xtmp.c_str()));

		begin = end;
		if (begin == std::string::npos) {
			break;
		} else {
			begin++;
		}
		end = tmp.find_first_of(',', begin);
	}
	return;
}

