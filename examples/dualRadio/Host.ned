// ***************************************************************************
// Project: Enabling Multiple Controllable Radios in OMNeT++ Nodes
//
// Copyright (C) 2010 Olafur Ragnar Helgason & Sylvia T. Kouyoumdjieva,
//               Laboratory for Communication Networks, KTH, Stockholm
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not see <http://www.gnu.org/licenses/>.
//
// ***************************************************************************

package org.mixim.examples.dualRadio;

import org.mixim.modules.nic.Nic80211Battery;
import org.mixim.modules.mobility.TraceMobility;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.base.modules.BaseUtility;
import org.mixim.base.modules.BaseNetwLayer;
import org.mixim.modules.nic.Nic802154_TI_CC2420;
import org.mixim.modules.netw.Arp;

module Host
{
    parameters:
        //string applType; //type of the application layer
        //string netwType; //type of the network layer
        //string mobType; //type of the mobility module
        @display("bgb=360,285,white");
    gates:
        input hpRadioIn;
        input lpRadioIn;
    submodules:
        blackboard: BaseUtility {
            parameters:
                @display("p=314,45;b=24,24,rect,black;i=block/blackboard");
        }
        arp: Arp {
            parameters:
                @display("p=238,45;b=24,24,rect,blue;i=block/process");
        }
        mobility: TraceMobility {
            parameters:
                @display("p=314,112;i=block/cogwheel");
        }
        battery: SimpleBattery {
            parameters:
                @display("p=238,112;b=30,25");
        }
        batteryStats: BatteryStats {
            parameters:
                @display("p=238,182;b=30,25");

        }
        appl: BeaconSender {
            parameters:
                @display("p=103,38;i=block/app");
        }

        primaryNet: BaseNetwLayer {
            parameters:
                @display("p=60,118;i=block/layer");
        }

        secondaryNet: BaseNetwLayer {
            parameters:
                @display("p=148,118;i=block/layer");
        }

        primaryNic: Nic802154_TI_CC2420 {
            parameters:
                @display("p=60,203;i=block/ifcard");
        }

        secondaryNic: HpNic {
            parameters:
                @display("p=148,203;i=block/ifcard");
        }


    connections allowunconnected:
        // Low power, primary radio
        primaryNet.upperGateOut --> appl.primaryDataIn;
        primaryNet.upperGateIn <-- appl.primaryDataOut;
        primaryNet.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.primaryControlIn;
        primaryNet.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.primaryControlOut;

        primaryNic.upperGateOut --> primaryNet.lowerGateIn;
        primaryNic.upperGateIn <-- primaryNet.lowerGateOut;
        primaryNic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> primaryNet.lowerControlIn;
        primaryNic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- primaryNet.lowerControlOut;

        lpRadioIn --> primaryNic.radioIn;

        // High power, secondary radio
        secondaryNet.upperGateOut --> appl.secondaryDataIn;
        secondaryNet.upperGateIn <-- appl.secondaryDataOut;
        secondaryNet.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.secondaryControlIn;
        secondaryNet.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.secondaryControlOut;

        secondaryNic.upperGateOut --> secondaryNet.lowerGateIn;
        secondaryNic.upperGateIn <-- secondaryNet.lowerGateOut;
        secondaryNic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> secondaryNet.lowerControlIn;
        secondaryNic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- secondaryNet.lowerControlOut;
        
        hpRadioIn --> secondaryNic.radioIn;
}
