//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
// @author Jan Holusa
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package ansa.routing.glbp;
import inet.applications.contract.IApp;

simple GLBPVirtualRouter like IApp
{
    parameters:
        @display("i=block/routing");
        //     	xml configData = default(xml("<GLBP />"));
        //     	string deviceId;     	
        string interfaceTableModule;
        string arp;
        //        string routingTableModule;// = default(".routingTableModule");
        int interface;
//		int vrid;

        //cisco jitter timers
        bool jittered = default(false);

        //default version of HSRP is 1
        int version = default(1);	 	 //default version of HSRP is 1
        int hellotime @unit(s) = default(3s);	  	//hellotime of actual device
        int holdtime @unit(s) = default(10s);	  	//holdtime of actual device
        //time during which VG redirect clients to secondary vf
        int redirect @unit(s) = default(600s);	//time during which VG redirect clients to secondary vf
        //after this time will be secondary vf unreacheable
        int timeout @unit(s) = default(14400s);	//after this time will be secondary vf unreacheable
        //priority of actual device
        int priority = default(100);	 	 //priority of actual device
        //weight of actual device
        int weight = default(100);		//weight of actual device
        //preemption flag
        bool preempt = default(false);	 //preemption flag
        //group number of actual device 	
        int group = default(0);	    	//group number of actual device 	
        //virtual IP address of actual HSRP device       	
        string virtualIP = default("0.0.0.0");	//virtual IP address of actual HSRP device       	

        int glbpUdpPort = default(3222);
        string glbpMulticastAddress = default("224.0.0.102");

    gates:
        input socketIn;	///to GLBP module
        output socketOut;  ///from GLBP module
}
