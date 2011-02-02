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

package org.mixim.modules.netw;

import org.mixim.base.modules.BaseArp;

// Module which is responsible for Addressresolution
simple Arp extends BaseArp
{
	parameters:
        @class(Arp);
    //parameters:
    //    bool notAffectedByHostState = true;
    //    bool coreDebug; // debug switch for the core framework
}
