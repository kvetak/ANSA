
package inet.examples.ansaExamples.cesnet.ospf;

import inet.ansa.ANSARouter;
import inet.world.ChannelInstaller;
import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;
import inet.world.ScenarioManager;
import inet.ansa.FormalAnalysis.CriticalnessAnalyzer;


network Network_E0
{
    @display("bgb=758,489");
    submodules:
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=673,25");
        }
        criticalnessAnalyzer: CriticalnessAnalyzer {
            parameters:
                @display("p=673,76");
        }
        R6_Brno: ANSARouter {
            @display("p=447,339");
            gates:

                pppg[6];
        }
        R9_CeskeBudejovice: ANSARouter {
            @display("p=184,356");
            gates:

                pppg[2];
        }
        R16_HradecKralove: ANSARouter {
            @display("p=386,125");
            gates:

                pppg[3];
        }
        R12_Cheb: ANSARouter {
            @display("p=32,136");
            gates:

                pppg[2];
        }
        R8_JHradec: ANSARouter {
            @display("p=256,327");
            gates:

                pppg[2];
        }
        R7_Jihlava: ANSARouter {
            @display("p=328,302");
            gates:

                pppg[2];
        }
        R5_KutnaHora: ANSARouter {
            @display("p=319,194");
            gates:

                pppg[1];
        }
        R23_Kyjov: ANSARouter {
            @display("p=515,379");
            gates:

                pppg[1];
        }
        R17_Letohrad: ANSARouter {
            @display("p=470,159");
            gates:

                pppg[3];
        }
        R15_Liberec: ANSARouter {
            @display("p=309,41");
            gates:

                pppg[2];
        }
        R11_MarLazne: ANSARouter {
            @display("p=91,194");
            gates:

                pppg[3];
        }
        R13_Most: ANSARouter {
            @display("p=130,90");
            gates:

                pppg[2];
        }
        R20_Olomouc: ANSARouter {
            @display("p=515,248");
            gates:

                pppg[3];
        }
        R18_Opava: ANSARouter {
            @display("p=602,171");
            gates:

                pppg[2];
        }
        R19_Ostrava: ANSARouter {
            @display("p=655,225");
            gates:

                pppg[3];
        }
        R4_Pardubice: ANSARouter {
            @display("p=386,182");
            gates:

                pppg[3];
        }
        R10_Plzen: ANSARouter {
            @display("p=145,248");
            gates:

                pppg[2];
        }
        R1_Podebrady: ANSARouter {
            @display("p=309,136");
            gates:

                pppg[1];
        }
        R2_Praha: ANSARouter {
            @display("p=226,171");
            gates:

                pppg[6];
        }
        R3_Tabor: ANSARouter {
            @display("p=226,279");
            gates:

                pppg[1];
        }
        R14_UstinL: ANSARouter {
            @display("p=217,64");
            gates:

                pppg[3];
        }
        R21_Zlin: ANSARouter {
            @display("p=575,339");
            gates:

                pppg[2];
        }
        R22_Znojmo: ANSARouter {
            @display("p=386,379");
            gates:

                pppg[1];
        }

    connections:
        R15_Liberec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R14_UstinL.pppg[1];
        R15_Liberec.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R16_HradecKralove.pppg[0];
        R11_MarLazne.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R10_Plzen.pppg[1];
        R11_MarLazne.pppg[1] <--> {  delay = 0.1us; datarate = 4Mbps; } <--> R2_Praha.pppg[0];
        R11_MarLazne.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R12_Cheb.pppg[0];
        R13_Most.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R12_Cheb.pppg[1];
        R13_Most.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R14_UstinL.pppg[0];
        R20_Olomouc.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R21_Zlin.pppg[0];
        R20_Olomouc.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R17_Letohrad.pppg[1];
        R20_Olomouc.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R19_Ostrava.pppg[1];
        R18_Opava.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R17_Letohrad.pppg[2];
        R18_Opava.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R19_Ostrava.pppg[0];
        R19_Ostrava.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R6_Brno.pppg[2];
        R4_Pardubice.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R2_Praha.pppg[5];
        R4_Pardubice.pppg[1] <--> {  delay = 0.1us; datarate = 10Mbps; } <--> R5_KutnaHora.pppg[0];
        R4_Pardubice.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R16_HradecKralove.pppg[1];
        R10_Plzen.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R9_CeskeBudejovice.pppg[1];
        R1_Podebrady.pppg[0] <--> {  delay = 0.1us; datarate = 40Mbps; } <--> R2_Praha.pppg[2];
        R2_Praha.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; } <--> R14_UstinL.pppg[2];
        R2_Praha.pppg[3] <--> {  delay = 0.1us; datarate = 50Mbps; } <--> R3_Tabor.pppg[0];
        R2_Praha.pppg[4] <--> {  delay = 0.1us; datarate = 10000Mbps; } <--> R6_Brno.pppg[1];
        R6_Brno.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R7_Jihlava.pppg[0];
        R6_Brno.pppg[3] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R21_Zlin.pppg[1];
        R6_Brno.pppg[4] <--> {  delay = 0.1us; datarate = 10Mbps; } <--> R23_Kyjov.pppg[0];
        R6_Brno.pppg[5] <--> {  delay = 0.1us; datarate = 10Mbps; } <--> R22_Znojmo.pppg[0];
        R9_CeskeBudejovice.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R8_JHradec.pppg[1];
        R16_HradecKralove.pppg[2] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R17_Letohrad.pppg[0];
        R8_JHradec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> R7_Jihlava.pppg[1];
}

network Network_E10
{
    @display("bgb=758,489");
    submodules:
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=673,56");
        }
        criticalnessAnalyzer: CriticalnessAnalyzer {
            parameters:
                @display("p=673,76");
        }
        R6_Brno: ANSARouter {
            @display("p=447,339");
            gates:

                pppg[6];
        }
        R9_CeskeBudejovice: ANSARouter {
            @display("p=184,356");
            gates:

                pppg[2];
        }
        R16_HradecKralove: ANSARouter {
            @display("p=386,125");
            gates:

                pppg[3];
        }
        R12_Cheb: ANSARouter {
            @display("p=32,136");
            gates:

                pppg[2];
        }
        R8_JHradec: ANSARouter {
            @display("p=256,327");
            gates:

                pppg[2];
        }
        R7_Jihlava: ANSARouter {
            @display("p=328,302");
            gates:

                pppg[2];
        }
        R5_KutnaHora: ANSARouter {
            @display("p=319,194");
            gates:

                pppg[1];
        }
        R23_Kyjov: ANSARouter {
            @display("p=515,379");
            gates:

                pppg[1];
        }
        R17_Letohrad: ANSARouter {
            @display("p=470,159");
            gates:

                pppg[3];
        }
        R15_Liberec: ANSARouter {
            @display("p=309,41");
            gates:

                pppg[2];
        }
        R11_MarLazne: ANSARouter {
            @display("p=91,194");
            gates:

                pppg[3];
        }
        R13_Most: ANSARouter {
            @display("p=130,90");
            gates:

                pppg[2];
        }
        R20_Olomouc: ANSARouter {
            @display("p=515,248");
            gates:

                pppg[3];
        }
        R18_Opava: ANSARouter {
            @display("p=602,171");
            gates:

                pppg[2];
        }
        R19_Ostrava: ANSARouter {
            @display("p=655,225");
            gates:

                pppg[3];
        }
        R4_Pardubice: ANSARouter {
            @display("p=386,182");
            gates:

                pppg[3];
        }
        R10_Plzen: ANSARouter {
            @display("p=145,248");
            gates:

                pppg[2];
        }
        R1_Podebrady: ANSARouter {
            @display("p=309,136");
            gates:

                pppg[1];
        }
        R2_Praha: ANSARouter {
            @display("p=226,171");
            gates:

                pppg[6];
        }
        R3_Tabor: ANSARouter {
            @display("p=226,279");
            gates:

                pppg[1];
        }
        R14_UstinL: ANSARouter {
            @display("p=217,64");
            gates:

                pppg[3];
        }
        R21_Zlin: ANSARouter {
            @display("p=575,339");
            gates:

                pppg[2];
        }
        R22_Znojmo: ANSARouter {
            @display("p=386,379");
            gates:

                pppg[1];
        }

    connections:
        R15_Liberec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R14_UstinL.pppg[1];
        R15_Liberec.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R16_HradecKralove.pppg[0];
        R11_MarLazne.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R10_Plzen.pppg[1];
        R11_MarLazne.pppg[1] <--> {  delay = 0.1us; datarate = 4Mbps; per = 0.1; } <--> R2_Praha.pppg[0];
        R11_MarLazne.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R12_Cheb.pppg[0];
        R13_Most.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R12_Cheb.pppg[1];
        R13_Most.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R14_UstinL.pppg[0];
        R20_Olomouc.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R21_Zlin.pppg[0];
        R20_Olomouc.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R17_Letohrad.pppg[1];
        R20_Olomouc.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R19_Ostrava.pppg[1];
        R18_Opava.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R17_Letohrad.pppg[2];
        R18_Opava.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R19_Ostrava.pppg[0];
        R19_Ostrava.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R6_Brno.pppg[2];
        R4_Pardubice.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R2_Praha.pppg[5];
        R4_Pardubice.pppg[1] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.1; } <--> R5_KutnaHora.pppg[0];
        R4_Pardubice.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R16_HradecKralove.pppg[1];
        R10_Plzen.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R9_CeskeBudejovice.pppg[1];
        R1_Podebrady.pppg[0] <--> {  delay = 0.1us; datarate = 40Mbps; per = 0.1; } <--> R2_Praha.pppg[2];
        R2_Praha.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.1; } <--> R14_UstinL.pppg[2];
        R2_Praha.pppg[3] <--> {  delay = 0.1us; datarate = 50Mbps; per = 0.1; } <--> R3_Tabor.pppg[0];
        R2_Praha.pppg[4] <--> {  delay = 0.1us; datarate = 10000Mbps; per = 0.1; } <--> R6_Brno.pppg[1];
        R6_Brno.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R7_Jihlava.pppg[0];
        R6_Brno.pppg[3] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R21_Zlin.pppg[1];
        R6_Brno.pppg[4] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.1; } <--> R23_Kyjov.pppg[0];
        R6_Brno.pppg[5] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.1; } <--> R22_Znojmo.pppg[0];
        R9_CeskeBudejovice.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R8_JHradec.pppg[1];
        R16_HradecKralove.pppg[2] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R17_Letohrad.pppg[0];
        R8_JHradec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.1; } <--> R7_Jihlava.pppg[1];
}

network Network_E02
{
    @display("bgb=758,489");
    submodules:
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=673,56");
        }
        criticalnessAnalyzer: CriticalnessAnalyzer {
            parameters:
                @display("p=673,76");
        }
        R6_Brno: ANSARouter {
            @display("p=447,339");
            gates:

                pppg[6];
        }
        R9_CeskeBudejovice: ANSARouter {
            @display("p=184,356");
            gates:

                pppg[2];
        }
        R16_HradecKralove: ANSARouter {
            @display("p=386,125");
            gates:

                pppg[3];
        }
        R12_Cheb: ANSARouter {
            @display("p=32,136");
            gates:

                pppg[2];
        }
        R8_JHradec: ANSARouter {
            @display("p=256,327");
            gates:

                pppg[2];
        }
        R7_Jihlava: ANSARouter {
            @display("p=328,302");
            gates:

                pppg[2];
        }
        R5_KutnaHora: ANSARouter {
            @display("p=319,194");
            gates:

                pppg[1];
        }
        R23_Kyjov: ANSARouter {
            @display("p=515,379");
            gates:

                pppg[1];
        }
        R17_Letohrad: ANSARouter {
            @display("p=470,159");
            gates:

                pppg[3];
        }
        R15_Liberec: ANSARouter {
            @display("p=309,41");
            gates:

                pppg[2];
        }
        R11_MarLazne: ANSARouter {
            @display("p=91,194");
            gates:

                pppg[3];
        }
        R13_Most: ANSARouter {
            @display("p=130,90");
            gates:

                pppg[2];
        }
        R20_Olomouc: ANSARouter {
            @display("p=515,248");
            gates:

                pppg[3];
        }
        R18_Opava: ANSARouter {
            @display("p=602,171");
            gates:

                pppg[2];
        }
        R19_Ostrava: ANSARouter {
            @display("p=655,225");
            gates:

                pppg[3];
        }
        R4_Pardubice: ANSARouter {
            @display("p=386,182");
            gates:

                pppg[3];
        }
        R10_Plzen: ANSARouter {
            @display("p=145,248");
            gates:

                pppg[2];
        }
        R1_Podebrady: ANSARouter {
            @display("p=309,136");
            gates:

                pppg[1];
        }
        R2_Praha: ANSARouter {
            @display("p=226,171");
            gates:

                pppg[6];
        }
        R3_Tabor: ANSARouter {
            @display("p=226,279");
            gates:

                pppg[1];
        }
        R14_UstinL: ANSARouter {
            @display("p=217,64");
            gates:

                pppg[3];
        }
        R21_Zlin: ANSARouter {
            @display("p=575,339");
            gates:

                pppg[2];
        }
        R22_Znojmo: ANSARouter {
            @display("p=386,379");
            gates:

                pppg[1];
        }

    connections:
        R15_Liberec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R14_UstinL.pppg[1];
        R15_Liberec.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R16_HradecKralove.pppg[0];
        R11_MarLazne.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R10_Plzen.pppg[1];
        R11_MarLazne.pppg[1] <--> {  delay = 0.1us; datarate = 4Mbps; per = 0.02; } <--> R2_Praha.pppg[0];
        R11_MarLazne.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R12_Cheb.pppg[0];
        R13_Most.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R12_Cheb.pppg[1];
        R13_Most.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R14_UstinL.pppg[0];
        R20_Olomouc.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R21_Zlin.pppg[0];
        R20_Olomouc.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R17_Letohrad.pppg[1];
        R20_Olomouc.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R19_Ostrava.pppg[1];
        R18_Opava.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R17_Letohrad.pppg[2];
        R18_Opava.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R19_Ostrava.pppg[0];
        R19_Ostrava.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R6_Brno.pppg[2];
        R4_Pardubice.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R2_Praha.pppg[5];
        R4_Pardubice.pppg[1] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.02; } <--> R5_KutnaHora.pppg[0];
        R4_Pardubice.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R16_HradecKralove.pppg[1];
        R10_Plzen.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R9_CeskeBudejovice.pppg[1];
        R1_Podebrady.pppg[0] <--> {  delay = 0.1us; datarate = 40Mbps; per = 0.02; } <--> R2_Praha.pppg[2];
        R2_Praha.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.02; } <--> R14_UstinL.pppg[2];
        R2_Praha.pppg[3] <--> {  delay = 0.1us; datarate = 50Mbps; per = 0.02; } <--> R3_Tabor.pppg[0];
        R2_Praha.pppg[4] <--> {  delay = 0.1us; datarate = 10000Mbps; per = 0.02; } <--> R6_Brno.pppg[1];
        R6_Brno.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R7_Jihlava.pppg[0];
        R6_Brno.pppg[3] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R21_Zlin.pppg[1];
        R6_Brno.pppg[4] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.02; } <--> R23_Kyjov.pppg[0];
        R6_Brno.pppg[5] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.02; } <--> R22_Znojmo.pppg[0];
        R9_CeskeBudejovice.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R8_JHradec.pppg[1];
        R16_HradecKralove.pppg[2] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R17_Letohrad.pppg[0];
        R8_JHradec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.02; } <--> R7_Jihlava.pppg[1];
}


network Network_E05
{
    @display("bgb=758,489");
    submodules:
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=673,56");
        }
        criticalnessAnalyzer: CriticalnessAnalyzer {
            parameters:
                @display("p=673,76");
        }
        R6_Brno: ANSARouter {
            @display("p=447,339");
            gates:

                pppg[6];
        }
        R9_CeskeBudejovice: ANSARouter {
            @display("p=184,356");
            gates:

                pppg[2];
        }
        R16_HradecKralove: ANSARouter {
            @display("p=386,125");
            gates:

                pppg[3];
        }
        R12_Cheb: ANSARouter {
            @display("p=32,136");
            gates:

                pppg[2];
        }
        R8_JHradec: ANSARouter {
            @display("p=256,327");
            gates:

                pppg[2];
        }
        R7_Jihlava: ANSARouter {
            @display("p=328,302");
            gates:

                pppg[2];
        }
        R5_KutnaHora: ANSARouter {
            @display("p=319,194");
            gates:

                pppg[1];
        }
        R23_Kyjov: ANSARouter {
            @display("p=515,379");
            gates:

                pppg[1];
        }
        R17_Letohrad: ANSARouter {
            @display("p=470,159");
            gates:

                pppg[3];
        }
        R15_Liberec: ANSARouter {
            @display("p=309,41");
            gates:

                pppg[2];
        }
        R11_MarLazne: ANSARouter {
            @display("p=91,194");
            gates:

                pppg[3];
        }
        R13_Most: ANSARouter {
            @display("p=130,90");
            gates:

                pppg[2];
        }
        R20_Olomouc: ANSARouter {
            @display("p=515,248");
            gates:

                pppg[3];
        }
        R18_Opava: ANSARouter {
            @display("p=602,171");
            gates:

                pppg[2];
        }
        R19_Ostrava: ANSARouter {
            @display("p=655,225");
            gates:

                pppg[3];
        }
        R4_Pardubice: ANSARouter {
            @display("p=386,182");
            gates:

                pppg[3];
        }
        R10_Plzen: ANSARouter {
            @display("p=145,248");
            gates:

                pppg[2];
        }
        R1_Podebrady: ANSARouter {
            @display("p=309,136");
            gates:

                pppg[1];
        }
        R2_Praha: ANSARouter {
            @display("p=226,171");
            gates:

                pppg[6];
        }
        R3_Tabor: ANSARouter {
            @display("p=226,279");
            gates:

                pppg[1];
        }
        R14_UstinL: ANSARouter {
            @display("p=217,64");
            gates:

                pppg[3];
        }
        R21_Zlin: ANSARouter {
            @display("p=575,339");
            gates:

                pppg[2];
        }
        R22_Znojmo: ANSARouter {
            @display("p=386,379");
            gates:

                pppg[1];
        }

    connections:
        R15_Liberec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R14_UstinL.pppg[1];
        R15_Liberec.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R16_HradecKralove.pppg[0];
        R11_MarLazne.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R10_Plzen.pppg[1];
        R11_MarLazne.pppg[1] <--> {  delay = 0.1us; datarate = 4Mbps; per = 0.05; } <--> R2_Praha.pppg[0];
        R11_MarLazne.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R12_Cheb.pppg[0];
        R13_Most.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R12_Cheb.pppg[1];
        R13_Most.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R14_UstinL.pppg[0];
        R20_Olomouc.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R21_Zlin.pppg[0];
        R20_Olomouc.pppg[1] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R17_Letohrad.pppg[1];
        R20_Olomouc.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R19_Ostrava.pppg[1];
        R18_Opava.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R17_Letohrad.pppg[2];
        R18_Opava.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R19_Ostrava.pppg[0];
        R19_Ostrava.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R6_Brno.pppg[2];
        R4_Pardubice.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R2_Praha.pppg[5];
        R4_Pardubice.pppg[1] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.05; } <--> R5_KutnaHora.pppg[0];
        R4_Pardubice.pppg[2] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R16_HradecKralove.pppg[1];
        R10_Plzen.pppg[0] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R9_CeskeBudejovice.pppg[1];
        R1_Podebrady.pppg[0] <--> {  delay = 0.1us; datarate = 40Mbps; per = 0.05; } <--> R2_Praha.pppg[2];
        R2_Praha.pppg[1] <--> {  delay = 0.1us; datarate = 1000Mbps; per = 0.05; } <--> R14_UstinL.pppg[2];
        R2_Praha.pppg[3] <--> {  delay = 0.1us; datarate = 50Mbps; per = 0.05; } <--> R3_Tabor.pppg[0];
        R2_Praha.pppg[4] <--> {  delay = 0.1us; datarate = 10000Mbps; per = 0.05; } <--> R6_Brno.pppg[1];
        R6_Brno.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R7_Jihlava.pppg[0];
        R6_Brno.pppg[3] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R21_Zlin.pppg[1];
        R6_Brno.pppg[4] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.05; } <--> R23_Kyjov.pppg[0];
        R6_Brno.pppg[5] <--> {  delay = 0.1us; datarate = 10Mbps; per = 0.05; } <--> R22_Znojmo.pppg[0];
        R9_CeskeBudejovice.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R8_JHradec.pppg[1];
        R16_HradecKralove.pppg[2] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R17_Letohrad.pppg[0];
        R8_JHradec.pppg[0] <--> {  delay = 0.1us; datarate = 100Mbps; per = 0.05; } <--> R7_Jihlava.pppg[1];
}
