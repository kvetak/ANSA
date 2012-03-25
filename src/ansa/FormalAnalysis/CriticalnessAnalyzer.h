//
// Copyright (C) 20 Martin Danko
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

#ifndef CRITICALNESSANALYZER_H
#define CRITICALNESSANALYZER_H

#include <omnetpp.h>
#include <fstream>


namespace CA {

class NeighborRecord {
    std::string name;
    double cost; 
public:
    NeighborRecord() {name = ""; cost = 0.0;}
    NeighborRecord(std::string n_name, double n_cost) {name = n_name; cost = n_cost;}
    
    std::string getName() {return name;}
    void setName(std::string n_name) {name = n_name;}
    
    double getCost() {return cost;}
    void setCost(double n_cost) {cost = n_cost;}
      
};

class RouterRecord {

private:
    cModule *pModule;
    std::string name;
    
    std::vector<NeighborRecord> neighbors;
    
    int criticalness;
    

public:
    RouterRecord() {name = ""; neighbors.clear(); criticalness = 0; pModule = NULL;}
    RouterRecord(std::string n_name, cModule *mod) {name = n_name; neighbors.clear(); criticalness = 0; pModule = mod;}
    
    cModule *getModule() {return pModule;}
    
    std::string getName() {return name;}
    void setName(std::string n_name) {name = n_name;}
    
    int getCriticalness() {return criticalness;}
    void setCriticalness(int n_criticalness) {criticalness = n_criticalness;}
    
    int getNeighborsNum() {return neighbors.size();}
    NeighborRecord getNeighbor(int i) {return neighbors.at(i);}
    void addNeighbor(NeighborRecord &n_neighbor) {neighbors.push_back(n_neighbor);}
};

inline std::ostream& operator<< (std::ostream& ostr, RouterRecord& rec)
{
    ostr << rec.getName() << " ... " ;
    ostr << " Criticalness: " << rec.getCriticalness() << "%";
    
    if(rec.getCriticalness() == 100)
      ostr << "  this node is Critical Point";
    if(rec.getCriticalness() == 0)
      ostr << "  this node is Universal Point";
    
    return ostr;
}


class Path {

private:
    std::vector<std::string> path;
    
    double cost;  

public:
    Path() {path.clear(); cost = 0.0; }
    
    
    double getCost() {return cost;}
    void setCost(double n_cost) {cost = n_cost;}
    
    std::vector<std::string> &getPath() {return path;}
    void addNode(std::string &node, double n_cost) {path.push_back(node); cost += n_cost;}
    void removeLastNode() {path.pop_back();}
    bool isInPath(std::string &node);
    
    
};

inline std::ostream& operator<< (std::ostream& ostr, Path& path)
{
    ostr << "Cost: " << path.getCost() << "     Path: ";
    std::vector<std::string> tp = path.getPath();
    
    for (std::vector<std::string>::iterator it = tp.begin(); it!=tp.end(); ++it)
    {
      if(it == tp.begin())
        ostr << *it;
      else
        ostr << " -> "<< *it;
    }  
    return ostr;
}

}

 
class CriticalnessAnalyzer : public cSimpleModule
{
  private:
  
  std::string startNode;
  std::string destNode;
  
  bool analyze;
  
  std::vector<CA::RouterRecord> routers;
  
  CA::Path tempPath;
  std::vector<CA::Path> paths;

  protected:
  
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    
  public:
  
    std::vector<CA::RouterRecord>::iterator getRouterRecord(std::string &name);
    void generatePaths(std::string &name);
    void sortByCost();
    void calculateCriticalness();
    bool existRouter(std::string &name);
    void updateDisplayString();
    void writeResultsToFile();
  

};

#endif
