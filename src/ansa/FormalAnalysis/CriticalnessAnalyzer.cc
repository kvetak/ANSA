//
// Copyright (C) 2010 Martin Danko
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

#include "CriticalnessAnalyzer.h"


Define_Module(CriticalnessAnalyzer);

void CriticalnessAnalyzer::initialize()
{

if(par("analyze"))
{
  cTopology topology;
  
  topology.extractByNedTypeName(cStringTokenizer("inet.ansa.ANSARouter").asVector());
  
  routers.clear();
  
  for(int i = 0; i < topology.getNumNodes(); ++i)
  {
    cTopology::Node *node = topology.getNode(i);
    
    CA::RouterRecord router(node->getModule()->getName(), node->getModule());
    
    for(int j = 0; j < node->getNumOutLinks(); ++j)
    {
      cTopology::LinkOut  *link = node->getLinkOut(j);
      double cost = (100000000 / link->getLocalGate()->getTransmissionChannel()->par("datarate").doubleValue());
      std::string name = link->getRemoteNode()->getModule()->getName();
      
      CA::NeighborRecord neigh(name, cost);
      router.addNeighbor(neigh);
    }
    
    routers.push_back(router);
    
  }

  startNode = (const char *) par("startNode");
  if(!existRouter(startNode))
    error("Error - Start node %s doesn't exist", startNode.c_str());
  
  destNode = (const char *) par("destNode");
  if(!existRouter(destNode))
    error("Error - Destination node %s doesn't exist", destNode.c_str());
  
  tempPath.addNode(startNode, 0.0);
  generatePaths(startNode);
  sortByCost();
  calculateCriticalness();
  updateDisplayString();
  writeResultsToFile();
  
  WATCH_VECTOR(paths);
  WATCH_VECTOR(routers);
}  
}

void CriticalnessAnalyzer::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

std::vector<CA::RouterRecord>::iterator  CriticalnessAnalyzer::getRouterRecord(std::string &name)
{
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
     if(name == it->getName())
      return it;
  }
  return routers.end();
}


void CriticalnessAnalyzer::generatePaths(std::string &name)
{
  std::vector<CA::RouterRecord>::iterator it = getRouterRecord(name);
  
  if(it !=routers.end())
  {
    for(int i = 0; i < it->getNeighborsNum(); ++i)
    {
      std::string neighName = it->getNeighbor(i).getName();
      double cost = it->getNeighbor(i).getCost();
      double actCost = tempPath.getCost(); 
      
      if (neighName == destNode)
      {
        tempPath.addNode(neighName, cost);
        paths.push_back(tempPath);
        tempPath.removeLastNode();
        tempPath.setCost(actCost);
      }
      else if (!tempPath.isInPath(neighName))
      {
        tempPath.addNode(neighName, cost);
        generatePaths(neighName);
        tempPath.removeLastNode();
        tempPath.setCost(actCost);
      }
    }
  }
  
}

void CriticalnessAnalyzer::sortByCost()
{
  std::vector<CA::Path> tmpPaths;
  
  while (paths.size() > 0)
  {
    int i = 0;
    int index = 0;
    double cost = paths.at(0).getCost();
    
    for (std::vector<CA::Path>::iterator it = paths.begin(); it!=paths.end(); ++it)
    {
      if(it->getCost() < cost)
      {
        cost = it->getCost();
        index = i;
      }
      ++i;
    }
    tmpPaths.push_back(paths.at(index));
    paths.erase(paths.begin() + index);   
  }
  
  paths = tmpPaths;
}

void CriticalnessAnalyzer::calculateCriticalness()
{
  int pathNum = paths.size();
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    int p = 0;
    std::string name = it->getName();
    
    
    for (std::vector<CA::Path>::iterator pit = paths.begin(); pit!=paths.end(); ++pit)
    {
      if(pit->isInPath(name))
        ++p;
    }
    it->setCriticalness((int)((p*100)/pathNum));
  }  
}

bool CriticalnessAnalyzer::existRouter(std::string &name)
{
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    if(it->getName() == name)
      return true;
  }
  return false;
}

void CriticalnessAnalyzer::updateDisplayString()
{
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {  
      int cr = it->getCriticalness();
      char buf[80];
      sprintf(buf, "Criticalness: %d%%", cr);
       
      if(cr == 100 )
      {
        it->getModule()->getDisplayString().setTagArg("i",1,"red");
      }
      else if (cr == 0)
      {
        it->getModule()->getDisplayString().setTagArg("i",1,"yellow");
      }
      it->getModule()->getDisplayString().setTagArg("t",0,buf);
  }

}

void CriticalnessAnalyzer::writeResultsToFile()
{ 
  std::ofstream outFile;
  outFile.open ("results/CPAnalysis.csv");
  
  outFile << ";TotCost";
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    outFile << ";"<< it->getName();
  }
   outFile << "\n";
  
  int i = 1;
  
  for (std::vector<CA::Path>::iterator pit = paths.begin(); pit!=paths.end(); ++pit)
  {
    outFile << i << ";" << pit->getCost();
    for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
    {
      std::string name = it->getName();
      if(pit->isInPath(name))
        outFile << ";x";
      else
        outFile << ";";
    }
    outFile << "\n";
    ++i;
  }
  
  outFile << "CP;";
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    if(it->getCriticalness() == 100)
      outFile << ";x";
    else
      outFile << ";";
  }
  outFile << "\n";
  
  outFile << "UP;";
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    if(it->getCriticalness() == 0)
      outFile << ";x";
    else
      outFile << ";";
  }
  outFile << "\n";
  
  outFile << "Criticalness;";
  for (std::vector<CA::RouterRecord>::iterator it = routers.begin(); it!=routers.end(); ++it)
  {
    outFile << ";" << it->getCriticalness() << "%";
  }
  outFile << "\n";
  
  outFile.close();
}


bool CA::Path::isInPath(std::string &node)
{
  for (std::vector<std::string>::iterator it = path.begin(); it!=path.end(); ++it)
  {
     if(node == *it)
      return true;
  }
  return false;
}


