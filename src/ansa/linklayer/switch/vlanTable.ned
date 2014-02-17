package inet.ansa.linklayer.switch;

simple VLANTable
{
    parameters:
        string configFile = default("");
        string switchID = default("");
        int portCount;
        @display("i=block/table2,lightblue");
}
