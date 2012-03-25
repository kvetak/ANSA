
package inet.ansa.qos;

import inet.networklayer.queue.OutputQueue;

//
// Drop-tail queue, to be used in network interfaces.
// Conforms to the OutputQueue interface.
//
simple WeightedFairQueue like OutputQueue
{
    parameters:
        int frameCapacity = default(100);
    gates:
        input in;
        output out;
}



