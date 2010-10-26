// 
// Copyright (C) 2008 Olafur Ragnar Helgason, Laboratory for Communication 
//                    Networks, KTH, Stockholm
//           (C)      Kristjan Valur Jonsson, Reykjavik University, Reykjavik
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

//
// Node factory module
//
// The node factory instantiates nodes dynamically during a simulation 
// run from a tracefile.
//
// @author  Kristjan V. Jonsson
// @author  Olafur R. Helgason
// @version 1.1 
//

package org.mixim.modules.mobility;

simple NodeFactory
{
  parameters:
    double scenarioSizeX @unit(m);   // x coordinates of scenario
    double scenarioSizeY @unit(m);   // y coordinates of scenario
    string traceFile;       // name of mobility tracefile
    string defaultNodeTypename; // default node type to create
    string defaultNodeNamePrefix; // default prefix for node names   
    bool stopAfterLastDestroy = default(true);
}

