#include "ansa/routing/ospfv3/process/OSPFv3LSA.h"

namespace inet{

bool OSPFv3RouterLinkState::update(const OSPFv3RouterLSA *lsa)
{
    bool different = differsFrom(lsa);
    (*this) = (*lsa);
    resetInstallTime();
    if (different) {
//        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}

bool OSPFv3RouterLinkState::differsFrom(const OSPFv3RouterLSA *routerLSA) const
{
    const OSPFv3LSAHeader& thisHeader = getHeader();
    const OSPFv3LSAHeader& lsaHeader = routerLSA->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((this->getNtBit() != routerLSA->getNtBit()) ||
                         (this->getEBit() != routerLSA->getEBit()) ||
                         (this->getBBit() != routerLSA->getBBit()) ||
                         (this->getVBit() != routerLSA->getVBit()) ||
                         (this->getXBit() != routerLSA->getXBit()) ||
                         (this->getRoutersArraySize() != routerLSA->getRoutersArraySize()));

        if (!differentBody) {
            unsigned int routersCount = this->getRoutersArraySize();
            for (unsigned int i = 0; i < routersCount; i++) {
                auto thisRouter = this->getRouters(i);
                auto lsaRouter = routerLSA->getRouters(i);
                bool differentLink = ((thisRouter.type != lsaRouter.type) ||
                                      (thisRouter.metric != lsaRouter.metric) ||
                                      (thisRouter.interfaceID != lsaRouter.interfaceID) ||
                                      (thisRouter.neighborInterfaceID != lsaRouter.neighborInterfaceID) ||
                                      (thisRouter.neighborRouterID != lsaRouter.neighborRouterID));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}

std::string OSPFv3RouterLinkState::dumpRouterLSA() const
{
    std::stringstream out;

    out << this->getHeader().getAdvertisingRouter() << "\t";
    out << this->getHeader().getLsaAge() << "\t";
    out << this->getHeader().getLsaSequenceNumber() << "\t";
    out << this->getHeader().getLinkStateID() << "\n";

    int count = this->getRoutersArraySize();
    out << "neighbor count: " << count << "\n";
    for(int i=0; i<count; i++) {
        OSPFv3RouterLSABody body = this->getRouters(i);
        out << "\tneighbor " << body.neighborRouterID << "\n";
    }

    return out.str();
}

unsigned int calculateLSASize(OSPFv3LSA *lsa)
{
    //OSPFv3LSAType lsaType = static_cast<OSPFv3LSAType>(lsa->getHeader().getLsaType());
    uint16_t code = lsa->getHeader().getLsaType();
    unsigned int lsaLength;

    switch(code){
        case ROUTER_LSA:
        {
            OSPFv3RouterLSA* routerLSA = dynamic_cast<OSPFv3RouterLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_ROUTER_LSA_HEADER_LENGTH;
            unsigned short linkCount = routerLSA->getRoutersArraySize();

            for (unsigned short i = 0; i < linkCount; i++) {
                const OSPFv3RouterLSABody& router = routerLSA->getRouters(i);
                lsaLength += OSPFV3_ROUTER_LSA_BODY_LENGTH;
            }
            break;
        }
        case NETWORK_LSA:
        {
            OSPFv3NetworkLSA* networkLSA = dynamic_cast<OSPFv3NetworkLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_NETWORK_LSA_HEADER_LENGTH;
            unsigned short attachedCount = networkLSA->getAttachedRouterArraySize();
            lsaLength += attachedCount*OSPFV3_NETWORK_LSA_ATTACHED_LENGTH;

            break;
        }
        case LINK_LSA:
        {
            OSPFv3LinkLSA* linkLSA = dynamic_cast<OSPFv3LinkLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_LINK_LSA_BODY_LENGTH;
            unsigned short linkCount = linkLSA->getNumPrefixes();
            lsaLength += linkCount*OSPFV3_LINK_LSA_PREFIX_LENGTH;

            break;
        }
    }

    return lsaLength;
}

}//namespace inet
