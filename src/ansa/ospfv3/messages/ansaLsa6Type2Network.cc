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

bool AnsaOspf6::NetworkLsa::Update(const OspfNetworkLsa6* lsa) {
   bool different = DiffersFrom(lsa);
   (*this) = (*lsa);
   ResetInstallTime();
   if (different){
      ClearNextHops();
      return true;
   }else{
      return false;
   }
}

bool AnsaOspf6::NetworkLsa::DiffersFrom(const OspfNetworkLsa6* networkLsa) const {

   const OspfLsaHeader6& lsaHeader = networkLsa->getHeader();

   if (((header_var.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE))
         || ((header_var.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE))
         || (header_var.getAdvertisingRouter() != lsaHeader.getAdvertisingRouter())
         || (header_var.getLsSequenceNumber() != lsaHeader.getLsSequenceNumber())){
      return true;


   }else if (options_var != networkLsa->getOptions()){
      return true;
   }else{

      unsigned int routerCount = attachedRouters_arraysize;
      for (unsigned int i = 0; i < routerCount; i++){
         if (attachedRouters_var[i] != networkLsa->getAttachedRouters(i)){
            return true;
         }
      }
   }

   return false;
}
