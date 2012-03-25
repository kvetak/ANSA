#ifndef __INET_ANSASTAT_H
#define __INET_ANSASTAT_H


namespace AnsaOSPF {

class Stat
{
private:
    unsigned long helloPacketSend;
    unsigned long helloPacketReceived; 
    unsigned long ospfPacketSend;
    unsigned long ospfPacketReceived; 

public:
    Stat();
    void AddHelloPacketSend()      {++helloPacketSend;}
    void AddHelloPacketReceived()  {++helloPacketReceived;}
    void AddOspfPacketSend()       {++ospfPacketSend;}
    void AddOspfPacketReceived()   {++ospfPacketReceived;}
    unsigned long GetHelloPacketSend()      {return helloPacketSend;}
    unsigned long GetHelloPacketReceived()  {return helloPacketReceived;}
    unsigned long GetOspfPacketSend()       {return ospfPacketSend;}
    unsigned long GetOspfPacketReceived()   {return ospfPacketReceived;}
};

} // namespace AnsaOSPF

#endif // __INET_ANSASTAT_H
