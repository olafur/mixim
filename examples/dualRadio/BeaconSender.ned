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

import org.mixim.modules.application.IDualRadioApp;

simple BeaconSender like IDualRadioApp
{
    parameters:
        @class(BeaconSender);       
        bool debug;
        int headerLength @unit("bit");
        double beaconPeriod @unit("s") = default(0.1s);
        bool hpAlwaysOn = default(false);        
         
    gates:
        input primaryDataIn; // from network layer
        output primaryDataOut; // to network layer
        input primaryControlIn; // control from network layer
        output primaryControlOut; // control to network layer
        
        input secondaryDataIn; // from network layer
        output secondaryDataOut; // to network layer
        input secondaryControlIn; // control from network layer
        output secondaryControlOut; // control to network layer
}
