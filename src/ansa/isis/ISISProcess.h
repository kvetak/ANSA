// FIT VUT 2012
//
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

#ifndef ISISPROCESS_H_
#define ISISPROCESS_H_

#include "ISIStypes.h"
class ISISProcess;

class ISISProcess :public cObject {
private:
    std::string processTag;               /*!< label identifying IS-IS routing process */

    std::vector<ISISinterface> *ISISIft;     /*!< vector of available interfaces */

    const char *netAddr;                    /*!<  OSI network address in simplified NSAP format */
    unsigned char *areaId;            /*!< first 3Bytes of netAddr as area ID */
    unsigned char *sysId;             /*!< next 6Bytes of NetAddr as system ID */
    unsigned char *NSEL;              /*!< last 1Byte of Netaddr as NSEL identifier */
    std::vector<ISISadj> adjL1Table;        /*!< table of L1 adjacencies */
    std::vector<ISISadj> adjL2Table;        /*!< table of L2 adjacencies */
    short isType;                           /*!< defines router IS-IS operational mode (L1,L2,L1L2) */
    std::vector<LSPrecord> L1LSP;           /*!< L1 LSP database */
    std::vector<LSPrecord> L2LSP;           /*!< L2 LSP database */

    unsigned long lspRefreshInter;          /*!< interval at which LSPs are refreshed, 1 to 65535 (default 900) */
    unsigned long lspMaxLifetime;           /*!< Specifies the value of the lifetime in the LSP header. Should be bigger than lspRefreshInter */

public:
    ISISProcess();
    virtual ~ISISProcess();
    std::string getProcessTag() const;
    void setProcessTag(std::string processTag);
    short getIsType() const;
    unsigned long getLspMaxLifetime() const;
    unsigned long getLspRefreshInter() const;
    void setIsType(short  isType);
    void setLspMaxLifetime(unsigned long  lspMaxLifetime);
    void setLspRefreshInter(unsigned long  lspRefreshInter);
    const char *getNetAddr();
    void setNetAddr(const char *netAddr);
    const unsigned char *getSysId() const;
    std::vector<ISISinterface> *getIsisIft() const;
    std::vector<ISISadj> getAdjL1Table() const;
    std::vector<ISISadj> getAdjL2Table() const;
    const unsigned char *getAreaId() const;
    std::vector<LSPrecord> getL1Lsp() const;
    std::vector<LSPrecord> getL2Lsp() const;
    const unsigned char *getNsel() const;
    void setIsisIft(std::vector<ISISinterface> *isisIft);
    void setAreaId(unsigned char *areaId);
    void setNsel(unsigned char *nsel);
    void setSysId(unsigned char *sysId);

};

#endif /* ISISPROCESS_H_ */
