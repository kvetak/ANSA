/*
 * NET.cc
 *
 *  Created on: Aug 8, 2016
 *      Author: imarek
 */

#include "ansa/networklayer/clns/NET.h"

#include "ansa/networklayer/isis/ISIStypes.h"

namespace inet{

NET::NET() {
    // TODO Auto-generated constructor stub

}
NET::NET(std::string net){
    unsigned int dots = 0;
        size_t found;

        //net address (in this module - not according to standard O:-) MUST have the following format:
        //49.0001.1921.6800.1001.00
        //IDI: 49 (private addressing)
        //AREA: 0001
        //systemID: 1921.6800.1001 from IP 192.168.1.1
        //NSEL: 00

        found = net.find_first_of(".");
        if (found != 2 || net.length() != 25)
        {
//            return false;
        }

        areaID = new unsigned char[ISIS_AREA_ID];
        systemID = new unsigned char[ISIS_SYSTEM_ID];
        nsel = new unsigned char[1];

        while (found != std::string::npos)
        {

            switch (found)
                {
                case 2:
                    dots++;
                    areaID[0] = (unsigned char) (atoi(net.substr(0, 2).c_str()));
//                    cout << "BEZ ATOI" << net.substr(0, 2).c_str() << endl;
                    break;
                case 7:
                    dots++;
                    areaID[1] = (unsigned char) (atoi(net.substr(3, 2).c_str()));
                    areaID[2] = (unsigned char) (atoi(net.substr(5, 2).c_str()));
                    break;
                case 12:
                    dots++;
                    systemID[0] = (unsigned char) (strtol(net.substr(8, 2).c_str(), NULL, 16));
                    systemID[1] = (unsigned char) (strtol(net.substr(10, 2).c_str(), NULL, 16));
                    break;
                case 17:
                    dots++;
                    systemID[2] = (unsigned char) (strtol(net.substr(13, 2).c_str(), NULL, 16));
                    systemID[3] = (unsigned char) (strtol(net.substr(15, 2).c_str(), NULL, 16));
                    break;
                case 22:
                    dots++;
                    systemID[4] = (unsigned char) (strtol(net.substr(18, 2).c_str(), NULL, 16));
                    systemID[5] = (unsigned char) (strtol(net.substr(20, 2).c_str(), NULL, 16));
                    break;
                default:
//                    return false;
                    break;

                }

            found = net.find_first_of(".", found + 1);
        }

        if (dots != 5)
        {
//            return false;
        }

        nsel[0] = (unsigned char) (atoi(net.substr(23, 2).c_str()));

        //49.0001.1921.6801.2003.00




//        this->nickname = this->sysId[ISIS_SYSTEM_ID - 1] + this->sysId[ISIS_SYSTEM_ID - 2] * 0xFF;



}

NET::~NET() {
    // TODO Auto-generated destructor stub
}




}//end of namespace inet
