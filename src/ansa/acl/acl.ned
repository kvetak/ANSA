package inet.ansa.acl;

simple acl
{
    parameters:
       string configFile;
       @display("i=block/network2");  
       
    gates:
        input ifIn[];
        output ifOut[];
        input toNetworkLayerIn[];
        output toNetworkLayerOut[];
}
