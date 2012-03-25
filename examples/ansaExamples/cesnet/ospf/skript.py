import sys

subor=file('scenario3.xml','r')

i = 0

ev_starts = []
convergence_time = []

for riadok in subor:
    stlpce= riadok.split()
    if (len(stlpce) > 1) and (stlpce[0] == "<at"):
        ev_starts.append(int(stlpce[1][3:stlpce[1].rfind('"')]))
subor.close()

    
subor=file('out.txt','r')

packet_send = 0
packet_received = 0

sent_time = {}
receive_list = {}
receive_delay = {}

index = 0;
event = ev_starts[index]
started = 0

for riadok in subor:
    stlpce= riadok.split()
    if (len(stlpce) > 1):
        if (len(stlpce) == 6) and (stlpce[1][0:8] == "appData-"):
            if (stlpce[0] == "Send:"):
                packet_send = packet_send + 1
                sent_time[stlpce[1]] = float(stlpce[5])
                receive_list[stlpce[1]] = 0
                if (float(event) <=  float(stlpce[5])):
                    started = 1;
                if (started == 1) and (index + 1 < len(ev_starts)) and (float(ev_starts[index + 1]) <=  float(stlpce[5])):
                    convergence_time.append(-1.0)
                    index = index + 1
                    if (index < len(ev_starts)):
                        event = ev_starts[index]
                    else:
                        event = 99999999
            if (stlpce[0] == "Arrive:"):
                packet_received = packet_received + 1
                receive_list[stlpce[1]] = 1
                receive_delay[stlpce[1]] = float(stlpce[5]) - sent_time[stlpce[1]]
                if (started == 1):
                    convergence_time.append(float(stlpce[5])-float(event))
                    started = 0
                    index = index + 1
                    if (index < len(ev_starts)):
                        event = ev_starts[index]
                    else:
                        event = 99999999
if (started == 1):
    convergence_time.append(-1.0)
subor.close()

if (len(sys.argv)>1) and (sys.argv[1] == "dropped"):
    print "============================================"
    print "Convergence time"
    print "============================================"
    index = 1;
    for cislo in convergence_time:
        if(cislo > 0.0):
            print "%d->%d:" % (index,index + 1),"%.6f"%(cislo)
        else:
            print "%d->%d:" % (index,index + 1),"infinite"
        index = index + 1;

    print
    print "============================================"
    print "Sent packets: ",packet_send
    print "Dropped packets: ",packet_send - packet_received
    print "============================================"

stage_delay = []
if (len(sys.argv)>1) and (sys.argv[1] == "delay"):
    index = 0;
    inserted = 0;
    event = 100
    rangelist = range(len(receive_list))
    for number in rangelist:
        if (sent_time["appData-" + str(number)] > float(event) + 18.0):
            if (sent_time["appData-" + str(number)] < float(event) + 19.0):
                if (receive_list["appData-" + str(number)] == 1) and (inserted == 0):
                    stage_delay.append(float(receive_delay["appData-" + str(number)]))
                    inserted = 1;
            else:
                if (index < len(ev_starts)):
                    event = ev_starts[index]
                else:
                    event = 99999999
                index = index + 1
                inserted = 0
    print
    print "============================================"
    print "Packet delay"
    print "============================================"
    index = 1;
    for cislo in stage_delay:
        print "Stage %d:" % (index),"%.6f"%(cislo)
        index = index + 1;


conv_droped = []

if (len(sys.argv)>1) and (sys.argv[1] == "dropped"):
    index = 0;
    droped = 0;
    event_st = float(ev_starts[index])
    event_end = float(ev_starts[index]) + convergence_time[index]

    rangelist = range(len(receive_list))

    for number in rangelist:
        if (sent_time["appData-" + str(number)] >= event_st):
            if (sent_time["appData-" + str(number)] <= event_end):
                if (receive_list["appData-" + str(number)] == 0):
                    droped = droped + 1
            else:
                conv_droped.append(droped)
                droped = 0
                index = index + 1
                if (index < len(convergence_time)):
                    event_st = float(ev_starts[index])
                    event_end = float(ev_starts[index]) + convergence_time[index]
                else:
                    event_st = 999999999.9
                    event_end = 999999999.9


    print
    print "============================================"
    print "Dropped packets"
    print "============================================"
    index = 1;
    for cislo in conv_droped:
        if(convergence_time[index-1] > 0.0):
            print "%d->%d:" % (index,index + 1),"%d"%(cislo)
        else:
            print "%d->%d:" % (index,index + 1),"all"
        index = index + 1;

print
