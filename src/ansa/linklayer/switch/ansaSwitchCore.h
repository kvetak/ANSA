
/*
 *
 * zdenek kraus
 *
 * switchCore
 *
 */
//ASDF
#ifndef __ANSASWITCHCORE_H__
#define __ANSASWITCHCORE_H__

#include <omnetpp.h>

#include <map>
#include <string>

#include "base/NotificationBoard.h"
#include "linklayer/common/MACAddress.h"
#include "linklayer/ethernet/Ethernet.h"

#include "linklayer/ethernet/EtherFrame_m.h"
#include "ansa/linklayer/switch/AnsaEtherFrame_m.h"

#include "ansa/linklayer/switch/macTable.h"
#include "ansa/linklayer/switch/vlanTable.h"
#include "ansa/linklayer/switch/stp/stp.h"


class ANSASwitchCore : public cSimpleModule
{
  public:


	/* switch core frame descriptor for internal representation unpacked frame */
	typedef struct s_frame_descriptor {
	  cPacket * payload; // relaying message
	  int VID; // frame's VLAN ID
	  int rPort; // reception port
	  VLANTable::tVIDPortList portList; // suggested(then applied) list of destination ports
	  inet::MACAddress src; // source MAC address of the original frame
	  inet::MACAddress dest; // destination MAC address of the original frame
	  int etherType; // EtherType from inet::EthernetIIFrame
	  std::string name; // for simulation frame name
	} tFrameDescriptor;

	inet::MACAddress getBridgeAddress();

  private:

  protected:
  inet::MACAddress bridgeGroupAddress;
  inet::MACAddress bridgeAddress;

  MACTable * addrTable;
  VLANTable * vlanTable;
  Stp * spanningTree;

  cMessage * currentMsg;

  int portCount;

  virtual void initialize(int stage);
  virtual int numInitStages() const {return 1;}
  virtual void handleMessage(cMessage * msg);
  virtual void finish();

  bool reception(tFrameDescriptor& frame, cMessage *msg);
  void relay(tFrameDescriptor& frame);
  void dispatch(tFrameDescriptor& frame);

  bool ingress(tFrameDescriptor& tmp, inet::EthernetIIFrame *frame, int rPort);
  bool ingress(tFrameDescriptor& tmp, AnsaEtherFrame *frame, int rPort);
  void egress(tFrameDescriptor& frame);

  void learn(tFrameDescriptor& frame);

  void processFrame(cMessage * msg);
  void handleIncomingFrame(inet::EthernetIIFrame *frame);

  void handleAndDispatchFrame(inet::EthernetIIFrame *frame, int inputport);
  void broadcastFrame(inet::EthernetIIFrame *frame, int inputport);

  void sinkMsg(cMessage *msg);
  void sinkDupMsg(cMessage *msg);

  void dispatchBPDU(cMessage *msg, int port);
  void deliverBPDU(tFrameDescriptor& frame);

 // void tagMsg(int _vlan);
  AnsaEtherFrame * tagMsg(inet::EthernetIIFrame * _frame, int _vlan);
  //void untagMsg();
  inet::EthernetIIFrame * untagMsg(AnsaEtherFrame * _frame);
};



inline std::ostream& operator<<(std::ostream& os, const ANSASwitchCore::tFrameDescriptor f) {
	os << "---FRAME DESCRIPTOR---" << std::endl;

	os << " Name: " << f.name << std::endl;
	os << "  Src: " << f.src << std::endl;
	os << " Dest: " << f.dest << std::endl;
	os << "  VID: " << f.VID << std::endl;
	os << "rPort: " << f.rPort << std::endl;
	os << "pList: " << f.portList << std::endl;
	os << "eType: " << f.etherType << std::endl;
	os << "pLoad: " << f.payload << std::endl;

	os << "---END OF DESCRIPTOR---" << std::endl;
	return os;
}






#endif



