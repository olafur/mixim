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
      bool interpolate = default(false);  // Interpolate between waypoints or not
      double updateInterval @unit("s") = default(0.1s);      
}

