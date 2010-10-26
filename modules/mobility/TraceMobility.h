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
#ifndef __TRACE_MOBILITY_INCLUDED__
#define __TRACE_MOBILITY_INCLUDED__

#include <omnetpp.h>
#include <BaseMobility.h>
#include "TraceEvents_m.h"


/**
 * @brief Waypoint data structure.
 *
 * Data structure for a single waypoint event. Used by the NodeFactory object for
 * storing cached waypoints until they are forwarded to a newly created TraceMobility
 * module.
 */
/*
struct WAYPOINT_EVENT
{
    int id;
    simtime_t time;
    double x;
    double y;
    double z;
    double speed;
    simtime_t timeAtDest;
};
*/

/**
 * Container for cached waypoint events read from a trace file.
 */
typedef std::list < SetDestEvent > waypointEventsList;

/**
 * @brief Trace mobility module. 
 *
 * Used in conjunction with the NodeFactory object to implement the
 * trace mobility model.
 *
 * Trace mobility uses externally generated traces or measurement 
 * results to control a nodes movement. The module is initialized by the 
 * factory at a certain location upon creation. It then supplies the module
 * with its waypoint list and an optional destroy event. 
 *
 * @version 1.0 
 * @author  Olafur R. Helgason
 * @author  Kristjan V. Jonsson
 */
class TraceMobility : public BaseMobility
{
  protected:
    /** @brief In interpolate mode the nodes interpolated between destinations
     * with in timesteps defined by the update period.  In non-interpolate mode
     * they just appear at the destinations at the time specified by the setdest 
     * events. */
    bool _interpolate;

    /** @brief The update interval in seconds */
    simtime_t _updateInterval;

    /** @brief location update event */
    cMessage *_updateEvent;
    SetDestEvent* _setdestEvent;

    /** @brief Current and target position of the host */
    Coord _currentPos, _targetPos;
    simtime_t _timeAtTarget;

    /** @brief The pending event list. Contains waypoint updates */
    waypointEventsList m_eventList;

    double _speed;
    int _numSteps, _step;
    Coord _stepTarget, _stepSize;

  public:
    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int);
    /** @brief Called when the module is destroyed */
    virtual void finish();
    /** @brief The message handler. */
    virtual void handleSelfMsg(cMessage * msg);

    /** @brief Initialize the waypoint event list. Called by the 
               trace factory object upon creation of the node */
    void initializeTrace(const waypointEventsList * eventList);

  protected:
    virtual void makeMove();
    void scheduleNextWaypointEvent();
    void setTarget(SetDestEvent* waypoint);
};

#endif /* __TRACE_MOBILITY_INCLUDED__ */
