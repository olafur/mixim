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


import org.mixim.modules.nic.INic;
import org.mixim.modules.nic.NicController;
import org.mixim.modules.mac.Mac80211Control;
import org.mixim.modules.phy.PhyLayerControl;

module HpNic like INic
{
    parameters:
        string connectionManagerName = "secondaryConnectionManager";
        double sleepCurrent @unit(mA);
        double rxCurrent @unit(mA);
        double decodingCurrentDelta @unit(mA);
        double txCurrent @unit(mA);
        double setupRxCurrent @unit(mA);
        double setupTxCurrent @unit(mA);
        double rxTxCurrent @unit(mA);
        double txRxCurrent @unit(mA);
        bool startsOn = default(true);
        
        @display("p=60,166;i=block/ifcard;bgb=232,263");

    gates:
        input upperGateIn;
        input upperControlIn;
        input radioIn;
        output upperGateOut;
        output upperControlOut;
        
    submodules:
        nicController: NicController {
            parameters:
                @display("p=111,53;i=block/wrxtx");
        }

        mac: Mac80211Control {
            parameters:
                @display("p=111,130;i=block/mac");
        }

        phy: PhyLayerControl {
            parameters:
                headerLength = 192bit;
                @display("p=111,205;i=block/layer");
        }

    connections:
        radioIn --> phy.radioIn;
        // phy to mac: data goes here
        phy.upperGateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerGateOut;
        phy.upperGateOut --> { @display("m=m,73,0,50,50"); } --> mac.lowerGateIn;
        // phy to mac: control goes here:
        phy.upperControlIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerControlOut;
        phy.upperControlOut --> { @display("m=m,73,0,50,50"); } --> mac.lowerControlIn;
        // mac to control: control goes here
        mac.upperControlIn <-- nicController.lowerControlOut;
        mac.upperControlOut --> nicController.lowerControlIn;
		// mac to control: data goes here
        mac.upperGateOut --> nicController.lowerGateIn;
        mac.upperGateIn <-- nicController.lowerGateOut;
        // control to upper layers: data and control
        nicController.upperControlIn <-- upperControlIn;
        nicController.upperControlOut --> upperControlOut;
        nicController.upperGateIn <-- upperGateIn;
        nicController.upperGateOut --> upperGateOut;
}
