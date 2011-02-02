// ***************************************************************************
// 
// OppoNet Project
// This file is a part of the opponet project, jointly managed by the
// Laboratory for Communications Networks (LCN) at KTH in Stockholm, Sweden
// and Reykjavik University, Iceland.
//
// Copyright (C) 2008 Olafur Ragnar Helgason, Laboratory for Communication 
//                    Networks, KTH, Stockholm
//           (C)      Kristjan Valur Jonsson, Reykjavik University, Reykjavik
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

//
// Trace mobility module.
//
// Used in conjunction with the NodeFactory object to implement the
// trace mobility model.
//
// Trace mobility uses externally generated traces or measurement 
// results to control a nodes movement. The module is initialized by the 
// factory at a certain location upon creation. It then supplies the module
// with its waypoint list and an optional destroy event. 
//
// @author  Olafur R. Helgason
// @author  Kristjan V. Jonsson
// @version 1.0 
//

package org.mixim.modules.mobility;

import org.mixim.base.modules.BaseMobility;

simple TraceMobility extends BaseMobility
{
  parameters:
      @class(TraceMobility);
      x = default(-1);
      y = default(-1);
      z = default(-1);
      int nodeId = default(-1);	// Id of the node in the mobility tracefile
      bool interpolate = default(false);  // Interpolate between waypoints or not
      double updateInterval @unit("s") = default(0.1s);      
}

