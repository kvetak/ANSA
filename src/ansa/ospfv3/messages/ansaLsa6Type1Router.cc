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

#include "ansaLsa6.h"

bool AnsaOspf6::RouterLsa::Update(const OspfRouterLsa6* lsa) {
   EV<< "-----------------------------------------------------\n";
   PrintLsaHeader6(lsa->getHeader(), ev.getOStream());
   EV << "-----------------------------------------------------\n";
   PrintLsaHeader6(this->getHeader(), ev.getOStream());
   EV << "-----------------------------------------------------\n";
   bool different = DiffersFrom(lsa);
   (*this) = (*lsa);
   ResetInstallTime();
   if (different){
      ClearNextHops();
      return true;
   } else{
      return false;
   }
}

bool AnsaOspf6::RouterLsa::DiffersFrom(const OspfRouterLsa6* routerLsa) const {

   const OspfLsaHeader6& lsaHeader = routerLsa->getHeader();

   if (((header_var.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE))
         || ((header_var.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE))
         || (header_var.getAdvertisingRouter() != lsaHeader.getAdvertisingRouter())
         || (header_var.getLsSequenceNumber() != lsaHeader.getLsSequenceNumber())){
      return true;


   }else if (V_VirtualLinkEndpoint_var != routerLsa->getV_VirtualLinkEndpoint()){
      return true;
   }else if (E_AsBoundaryRouter_var != routerLsa->getE_AsBoundaryRouter()){
      return true;
   }else if (B_AreaBorderRouter_var != routerLsa->getB_AreaBorderRouter()){
      return true;
   }else if (options_var != routerLsa->getOptions()){
      return true;
   }else if (links_arraysize != routerLsa->getLinksArraySize()){
      return true;

   }else{

      unsigned int linkCount = links_arraysize;
      for (unsigned int i = 0; i < linkCount; i++){
         if (links_var[i] != routerLsa->getLinks(i)){
            return true;
         }
      }
   }

   return false;
}
