//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

network net_25
{
    parameters:
        double alpha = default(0.5);
        int availableJobNum = default(10); // 네트워크에서 실행시킬 수 있는 최재 job 개수
        int jobWaitingQueueLength = default(15);
        int nodeNum = default(25);
        int jobGenerateRate = default(20); // 10 퍼센트
        int modelNum = default(12);
        int linkCapacity = default(5);
        int nodeCapacity = default(5);
        int episode_length = default(100);
        bool reset_network = default(true);
        double low = default(0.05);
        double high = default(0.15);
    //@display("bgb=1000,1000");
    types:
        channel Channel extends ned.DelayChannel
        {
            delay = 100ms;
        }
    submodules:
        dataplane: dataplane {
            @display("p=300,50");
        }
        iot[nodeNum]: iot {
            @display("p=100,200,ri,200,200");
        }



    connections:
//        for i=0..nodeNum-1, for j=i..nodeNum-1, if i!=j && uniform(0,1) < alpha {
//            iot[i].out++ --> iot[j].in++;
//            iot[i].in++ <-- iot[j].out++;
//        }
//

	iot[0].out++ --> iot[1].in++;
    iot[0].in++ <-- iot[1].out++;
    
    iot[0].out++ --> iot[2].in++;
    iot[0].in++ <-- iot[2].out++;
    
    iot[0].out++ --> iot[3].in++;
    iot[0].in++ <-- iot[3].out++;
    
    iot[0].out++ --> iot[4].in++;
    iot[0].in++ <-- iot[4].out++;
    
    iot[0].out++ --> iot[6].in++;
    iot[0].in++ <-- iot[6].out++;
    
    iot[0].out++ --> iot[8].in++;
    iot[0].in++ <-- iot[8].out++;
    
    iot[0].out++ --> iot[9].in++;
    iot[0].in++ <-- iot[9].out++;
    
    iot[0].out++ --> iot[10].in++;
    iot[0].in++ <-- iot[10].out++;
    
    iot[0].out++ --> iot[12].in++;
    iot[0].in++ <-- iot[12].out++;
    
    iot[0].out++ --> iot[16].in++;
    iot[0].in++ <-- iot[16].out++;
    
    iot[0].out++ --> iot[19].in++;
    iot[0].in++ <-- iot[19].out++;
    
    iot[0].out++ --> iot[23].in++;
    iot[0].in++ <-- iot[23].out++;
    
    iot[0].out++ --> iot[24].in++;
    iot[0].in++ <-- iot[24].out++;
    
    iot[1].out++ --> iot[4].in++;
    iot[1].in++ <-- iot[4].out++;
    
    iot[1].out++ --> iot[5].in++;
    iot[1].in++ <-- iot[5].out++;
    
    iot[2].out++ --> iot[3].in++;
    iot[2].in++ <-- iot[3].out++;
    
    iot[2].out++ --> iot[5].in++;
    iot[2].in++ <-- iot[5].out++;
    
    iot[2].out++ --> iot[11].in++;
    iot[2].in++ <-- iot[11].out++;
    
    iot[2].out++ --> iot[14].in++;
    iot[2].in++ <-- iot[14].out++;
    
    iot[2].out++ --> iot[18].in++;
    iot[2].in++ <-- iot[18].out++;
    
    iot[3].out++ --> iot[7].in++;
    iot[3].in++ <-- iot[7].out++;
    
    iot[4].out++ --> iot[9].in++;
    iot[4].in++ <-- iot[9].out++;
    
    iot[4].out++ --> iot[12].in++;
    iot[4].in++ <-- iot[12].out++;
    
    iot[5].out++ --> iot[6].in++;
    iot[5].in++ <-- iot[6].out++;
    
    iot[5].out++ --> iot[8].in++;
    iot[5].in++ <-- iot[8].out++;
    
    iot[5].out++ --> iot[17].in++;
    iot[5].in++ <-- iot[17].out++;
    
    iot[6].out++ --> iot[7].in++;
    iot[6].in++ <-- iot[7].out++;
    
    iot[7].out++ --> iot[11].in++;
    iot[7].in++ <-- iot[11].out++;
    
    iot[7].out++ --> iot[13].in++;
    iot[7].in++ <-- iot[13].out++;
    
    iot[7].out++ --> iot[15].in++;
    iot[7].in++ <-- iot[15].out++;
    
    iot[8].out++ --> iot[15].in++;
    iot[8].in++ <-- iot[15].out++;
    
    iot[8].out++ --> iot[16].in++;
    iot[8].in++ <-- iot[16].out++;
    
    iot[8].out++ --> iot[22].in++;
    iot[8].in++ <-- iot[22].out++;
    
    iot[9].out++ --> iot[10].in++;
    iot[9].in++ <-- iot[10].out++;
    
    iot[9].out++ --> iot[13].in++;
    iot[9].in++ <-- iot[13].out++;
    
    iot[9].out++ --> iot[19].in++;
    iot[9].in++ <-- iot[19].out++;
    
    iot[9].out++ --> iot[23].in++;
    iot[9].in++ <-- iot[23].out++;
    
    iot[9].out++ --> iot[24].in++;
    iot[9].in++ <-- iot[24].out++;
    
    iot[10].out++ --> iot[14].in++;
    iot[10].in++ <-- iot[14].out++;
    
    iot[10].out++ --> iot[20].in++;
    iot[10].in++ <-- iot[20].out++;
    
    iot[10].out++ --> iot[21].in++;
    iot[10].in++ <-- iot[21].out++;
    
    iot[10].out++ --> iot[22].in++;
    iot[10].in++ <-- iot[22].out++;
    
    iot[14].out++ --> iot[18].in++;
    iot[14].in++ <-- iot[18].out++;
    
    iot[14].out++ --> iot[21].in++;
    iot[14].in++ <-- iot[21].out++;
    
    iot[15].out++ --> iot[20].in++;
    iot[15].in++ <-- iot[20].out++;
    
    iot[16].out++ --> iot[17].in++;
    iot[16].in++ <-- iot[17].out++;
        
        for i=0..nodeNum-1 {
            dataplane.out++ --> { @display("ls=,0"); } --> iot[i].in++;
            dataplane.in++ <-- { @display("ls=,0"); } <-- iot[i].outDataplane;
        }




//        for i=0..4 {
//        	agent.out++ --> Channel { @display("ls=,0"); } --> iot[i].in++;
//        }
//        
//        iot[0].out++ --> Channel --> iot[1].in++;
//        iot[0].in++ <-- Channel <-- iot[1].out++;
//        
//        iot[0].out++ --> Channel --> iot[2].in++;
//        iot[0].in++ <-- Channel <-- iot[2].out++;
//        
//        iot[1].out++ --> Channel --> iot[2].in++;
//        iot[1].in++ <-- Channel <-- iot[2].out++;
//        
//        iot[1].out++ --> Channel --> iot[3].in++;
//        iot[1].in++ <-- Channel <-- iot[3].out++;
//        
//        iot[2].out++ --> Channel --> iot[3].in++;
//        iot[2].in++ <-- Channel <-- iot[3].out++;
//        
//        iot[2].out++ --> Channel --> iot[4].in++;
//        iot[2].in++ <-- Channel <-- iot[4].out++;
//        
//        iot[3].out++ --> Channel --> iot[4].in++;
//        iot[3].in++ <-- Channel <-- iot[4].out++;


}
