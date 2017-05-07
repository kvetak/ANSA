#include "ansa/routing/ospfv3/process/OSPFv3LSA.h"

namespace inet{

OSPFv3SPFVertex::OSPFv3SPFVertex(VertexType type, VertexID vertexID)
{
    this->type = type;
    this->distance = 0;
    this->lsa = new struct VertexLSA;
    this->lsa->routerLSA = nullptr;
    this->lsa->networkLSA = nullptr;
    this->parent = nullptr;
    this->vertexID = vertexID;
    this->vertexSource = FLOODED;
}

void OSPFv3SPFVertex::setVertexLSA(OSPFv3LSA* lsa)
{
    if(this->type == ROUTER_VERTEX) {
        this->lsa->routerLSA = dynamic_cast<OSPFv3RouterLSA*>(lsa);
        if(this->lsa->routerLSA == nullptr)
            EV_DEBUG << "Unable to dynamic cast router lsa into vertex\n";
    }
    else {
        this->lsa->networkLSA = dynamic_cast<OSPFv3NetworkLSA*>(lsa);
        if(this->lsa->networkLSA == nullptr)
        EV_DEBUG << "Unable to dynamic cast network lsa into vertex\n";
    }
}

void OSPFv3SPFVertex::vertexInfo()
{
    EV_INFO << "Vertex:\n\tVertex Type: ";
    if(this->getVertexType()==ROUTER_VERTEX) {
        EV_INFO << "ROUTER\n\tVertex ID: ";
        EV_INFO << this->getVertexID().routerID << "\n";
    }
    else{
        EV_INFO << "NETWORK\n\tVertex ID: ";
        EV_INFO << this->getVertexID().interfaceID << ", " << this->getVertexID().routerID << "\n";
    }

    EV_INFO << "\tVertex Origin: ";
    if(this->getSource()==FLOODED)
        EV_INFO << "FLOODED\n";
    else
        EV_INFO << "ORIGINATED\n";

    EV_INFO << "\tVertex Distance: " << this->getDistance() << "\n";

    if(this->getParent() != nullptr) {
        EV_INFO << "\tVertex Parent: ";
        if(this->getParent()->getVertexType()==ROUTER_VERTEX)
            EV_INFO << this->getParent()->getVertexID().routerID << "\n";
        else
            EV_INFO << this->getParent()->getVertexID().interfaceID << ", " << this->getParent()->getVertexID().routerID << "\n";
    }
}

bool OSPFv3RouterNode::update(const OSPFv3RouterLSA *lsa)
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

bool OSPFv3RouterNode::differsFrom(const OSPFv3RouterLSA *routerLSA) const
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

std::string OSPFv3RouterNode::dumpRouterLSA() const
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
            lsaLength += linkCount*OSPFV3_ROUTER_LSA_BODY_LENGTH;

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
        case INTER_AREA_PREFIX_LSA:
        {
            OSPFv3InterAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3InterAreaPrefixLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH;

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
        case INTRA_AREA_PREFIX_LSA:
        {
            OSPFv3IntraAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3IntraAreaPrefixLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH;
            unsigned short prefixCount = prefixLSA->getNumPrefixes();
            lsaLength += prefixCount*OSPFV3_INTRA_AREA_PREFIX_LSA_PREFIX_LENGTH;

            break;
        }
    }

    return lsaLength;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSAHeader& lsaHeader)
{
    ostr << "LSAHeader: age=" << lsaHeader.getLsaAge()
         << ", type=";
    switch (lsaHeader.getLsaType()) {
        case ROUTER_LSA:
            ostr << "RouterLSA";
            break;

        case NETWORK_LSA:
            ostr << "NetworkLSA";
            break;

        case INTER_AREA_PREFIX_LSA:
            ostr << "SummaryLSA_Networks";
            break;

        case INTER_AREA_ROUTER_LSA:
            ostr << "SummaryLSA_ASBoundaryRouters";
            break;

        case AS_EXTERNAL_LSA:
            ostr << "ASExternalLSA";
            break;

        default:
            ostr << "Unknown";
            break;
    }
    ostr << ", LSID=" << lsaHeader.getLinkStateID().str(false)
         << ", advertisingRouter=" << lsaHeader.getAdvertisingRouter().str(false)
         << ", seqNumber=" << lsaHeader.getLsaSequenceNumber()
         << endl;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3NetworkLSA& lsa)
{
    ostr << "Mask: 64" ;//<< lsa.getNetMask();
    unsigned int cnt = lsa.getAttachedRouterArraySize();
    if (cnt) {
        ostr << ", Attached routers:";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << lsa.getAttachedRouter(i);
        }
    }
    ostr << ", " << lsa.getHeader();
    return ostr;
}

//std::ostream& operator<<(std::ostream& ostr, const TOSData& tos)
//{
//    ostr << "tos: " << (int)tos.tos
//         << "metric:";
//    for (int i = 0; i < 3; i++)
//        ostr << " " << (int)tos.tosMetric[i];
//    return ostr;
//}

//std::ostream& operator<<(std::ostream& ostr, const Link& link)
//{
//    ostr << "ID: " << link.getLinkID().str(false)
//         << ", data: ";
//    unsigned long data = link.getLinkData();
//    if ((data & 0xFF000000) != 0)
//        ostr << IPv4Address(data).str(false);
//    else
//        ostr << data;
//    ostr << ", cost: " << link.getLinkCost();
//    unsigned int cnt = link.getTosDataArraySize();
//    if (cnt) {
//        ostr << ", tos: {";
//        for (unsigned int i = 0; i < cnt; i++) {
//            ostr << " " << link.getTosData(i);
//        }
//        ostr << "}";
//    }
//    return ostr;
//}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3RouterLSA& lsa)
{
    if (lsa.getVBit())
        ostr << "V, ";
    if (lsa.getEBit())
        ostr << "E, ";
    if (lsa.getBBit())
        ostr << "B, ";
    ostr << "numberOfLinks: " << lsa.getRoutersArraySize() << ", ";
    unsigned int cnt = lsa.getRoutersArraySize();
    if (cnt) {
        ostr << "Links: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " {" << lsa.getRouters(i).neighborRouterID << "}";
        }
        ostr << "}, ";
    }
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3InterAreaPrefixLSA& lsa)
{
    ostr << "Mask: " << lsa.getPrefixLen()
         << ", Cost: " << lsa.getMetric() << ", ";
//    unsigned int cnt = lsa.getTosDataArraySize();
//    if (cnt) {
//        ostr << ", tosData: {";
//        for (unsigned int i = 0; i < cnt; i++) {
//            ostr << " " << lsa.getTosData(i);
//        }
//        ostr << "}, ";
//    }
    ostr << lsa.getHeader();
    return ostr;
}

//std::ostream& operator<<(std::ostream& ostr, const ExternalTOSInfo& tos)
//{
//    ostr << "TOSData: {" << tos.tosData
//         << "}, MetricType: " << tos.E_ExternalMetricType
//         << ", fwAddr: " << tos.forwardingAddress
//         << ", extRouteTag: " << tos.externalRouteTag;
//    return ostr;
//}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3ASExternalLSA& lsa)
{
    if (lsa.getEBit())
        ostr << "E, ";
    if (lsa.getTBit())
        ostr << "T, ";
    if (lsa.getFBit())
        ostr << "F, ";

    ostr << "Referenced LSA Type: " << lsa.getReferencedLSType()
         << ", Cost: " << lsa.getMetric()
         << ", Forward: " << lsa.getForwardingAddress()
         << ", ExtRouteTag: " << lsa.getExternalRouteTag()
         << ", Referenced LSID: " << lsa.getReferencedLSID()
         << ", ";
//    unsigned int cnt = contents.getExternalTOSInfoArraySize();
//    if (cnt) {
//        ostr << ", tosData: {";
//        for (unsigned int i = 0; i < cnt; i++) {
//            ostr << " " << contents.getExternalTOSInfo(i);
//        }
//        ostr << "}, ";
//    }
    ostr << lsa.getHeader();
    return ostr;
}

}//namespace inet
