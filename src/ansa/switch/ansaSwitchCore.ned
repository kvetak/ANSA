package inet.ansa.switch;

simple ANSASwitchCore
{
    parameters:
        @display("i=block/switch;is=vl");
    gates:
        input ifIn[];
        output ifOut[];
        input stpIn[];
        output stpOut[];
        output toSink;
}
