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

import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.modules.mobility.NodeFactory;

network TestDualRadio
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)        

        @display("bgb=329,250,white;bgp=0,0");
    submodules:
        primaryConnectionManager: SimpleConnectionManager {
            parameters:
                @display("p=130,32;b=42,42,rect,green;i=abstract/multicast");
        }
        secondaryConnectionManager: SimpleConnectionManager {
            parameters:
                @display("p=150,32;b=42,42,rect,green;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=31,34;i=misc/globe");
        }
        nodeFactory: NodeFactory {
            parameters:
                scenarioSizeX = playgroundSizeX;
                scenarioSizeY = playgroundSizeY;
                @display("p=31,129;i=eye14");
        }
    connections allowunconnected:

}
