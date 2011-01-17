// 
// Copyright (C) 2008 Olafur Ragnar Helgason, Laboratory for Communication 
//                    Networks, KTH, Stockholm
//           (C)      Kristjan Valur Jonsson, Reykjavik University, Reykjavik
//

#ifndef __TRACE_MOBILITY_INCLUDED__
#define __TRACE_MOBILITY_INCLUDED__

#include <omnetpp.h>
#include <BaseMobility.h>
#include "TraceEv_m.h"


/**
 * Container for cached waypoint events read from a trace file.
 */
typedef std::list<SetDestEv> waypointEventsList;

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
    /** @brief The ID of the node in the mobility tracefile */
    int nodeId;

    /** @brief In interpolate mode the nodes interpolated between destinations
     * with in timesteps defined by the update period.  In non-interpolate mode
     * they just appear at the destinations at the time specified by the setdest 
     * events. */
    bool interpolate;

    /** @brief The update interval in seconds */
    bool moving;
    
    /** @brief The update interval in seconds */
    simtime_t updateInterval;

    /** @brief location update event */
    cMessage* updateEvent;

    /** @brief Target position of the host */
    Coord targetPos;
    simtime_t timeAtTarget;

    /** @brief The pending event list. Contains waypoint updates */
    waypointEventsList eventList;

    double speed;
    int numSteps, step;
    Coord stepTarget, stepSize;

  public:
    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int);
    /** @brief Called when the module is destroyed */
    virtual void finish();
    /** @brief The message handler. */
    virtual void handleSelfMsg(cMessage* msg);

    int getNodeId() const {return nodeId;}

    /** @brief Initialize the waypoint event list. Called by the 
               trace factory object upon creation of the node */
    void initializeTrace(const waypointEventsList* eventList);

  protected:
    virtual void makeMove();
    void scheduleNextWaypoint();
    void startMoving();
    //void setTarget(double x, double y, simtime_t timeAtDest);
};

#endif /* __TRACE_MOBILITY_INCLUDED__ */
