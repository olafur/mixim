// 
// Copyright (C) 2008 Olafur Ragnar Helgason, Laboratory for Communication 
//                    Networks, KTH, Stockholm
//           (C)      Kristjan Valur Jonsson, Reykjavik University, Reykjavik
//

#include "TraceMobility.h"
#include <FWMath.h>

Define_Module(TraceMobility);

void TraceMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);
    if (stage == 0) {
        moving = false;
        interpolate = par("interpolate").boolValue();
        if (interpolate) {
            updateInterval = par("updateInterval");
        }
        updateEvent = new cMessage("updateEvent");
        // we want move-update events to have a higher priority than any other
        // events scheduled simultaneously.  This ensures that position is always
        // updated before other events.
        updateEvent->setSchedulingPriority(-1);
    } else if (stage == 1) {
        ev << "TraceMobility initialized at "<< move.getStartPos().info() << endl;
    }
}

void TraceMobility::finish()
{
    BaseMobility::finish();
    cancelAndDelete(updateEvent);
}

void TraceMobility::handleSelfMsg(cMessage* msg)
{
    if (msg == updateEvent) {
        if(moving) {
            makeMove();
            updatePosition();
        } else {
            startMoving();
        }
    } else {
        BaseMobility::handleMessage(msg);
    }
}

void TraceMobility::makeMove()
{
    step++;
    if (step == numSteps) {
        // Destination reached    
        move.setStart(targetPos);
        move.setSpeed(0);     
        move.setDirectionByVector(Coord(0,0));
        ev << "Target " << targetPos.info()
                   << " reached, scheduling next waypoint" << endl;
        moving = false;
        scheduleNextWaypoint();
    } else if (step < numSteps) {
        // step forward
        move.setStart(stepTarget);
        stepTarget += stepSize;
        if (updateEvent->isScheduled()) {
            cancelEvent(updateEvent);
        }
        if (numSteps - step == 1) {
            scheduleAt(timeAtTarget, updateEvent);
        } else {
            scheduleAt(simTime() + updateInterval, updateEvent);
        }
    } else {
        throw cRuntimeError("step > numSteps in TraceMobility");
    }
}

void TraceMobility::scheduleNextWaypoint()
{
    if (!eventList.empty()) {
        SetDestEv waypoint = eventList.front();
        if(updateEvent->isScheduled()) {
            throw cRuntimeError("Update event is already scheduled");
        }
        scheduleAt(waypoint.getTime(), updateEvent);
    }
}

void TraceMobility::startMoving()
{
    if (eventList.empty()) {
        throw cRuntimeError("No mobility events available!");
    }
    SetDestEv waypoint = eventList.front();
    eventList.pop_front();
    simtime_t now = simTime();
    if (waypoint.getTime() != now) {
        throw cRuntimeError("waypoint.getTime() != now");
    }
    if (waypoint.getTimeAtDest() < now) {
        throw cRuntimeError("timeAtDest (%s) is in the past (now = %s)",
                waypoint.getTimeAtDest().str().c_str(), now.str().c_str());
    }
    // TODO implement 3D support
    targetPos = Coord(waypoint.getX(), waypoint.getY());
    timeAtTarget = waypoint.getTimeAtDest();
    step = 0;
    if (targetPos == move.getStartPos()) {
        numSteps = 1;
        speed = 0;
        moving = false;
        scheduleNextWaypoint();
    } else {
        Coord pos = move.getStartPos();
        speed = pos.distance(targetPos) / (timeAtTarget - now);
        //if( (waypoint->getSpeed() - _speed)*(waypoint->getSpeed() - _speed) > 0.01 ) {
        //    ev << "waypoint->getSpeed()=" << waypoint->getSpeed() 
        //          << ", _speed=" << _speed << endl;
        //    throw cRuntimeError("Speed mismatch in tracefile");
        //}
        move.setSpeed(speed);
        move.setDirectionByTarget(targetPos);
        moving = true;
        // calculate steps for interpolate or non-interpolate
        simtime_t nextEventTime;
        if (!interpolate) {
            numSteps = 1;
            nextEventTime = timeAtTarget;
        } else {
            // Get the number of steps needed to be covered.
            simtime_t travelTime = timeAtTarget - now;
            numSteps = static_cast < int >(ceil(travelTime.dbl() / updateInterval));
            if (numSteps > 1) {
                stepSize = (targetPos - move.getStartPos()) / numSteps;
                stepTarget = move.getStartPos() + stepSize;
                nextEventTime = now + updateInterval;
            } else {
                nextEventTime = timeAtTarget;
            }
        }
        if (updateEvent->isScheduled()) {
            cancelEvent(updateEvent);
        }
        scheduleAt(nextEventTime, updateEvent);
        ev << "Start moving from "<< move.getStartPos().info() << " to " 
           << targetPos.info() << " in " << numSteps << " steps "
           << " at speed=" << speed << endl;
    }
}

void TraceMobility::initializeTrace(const waypointEventsList* el)
{
    Enter_Method_Silent();
    if (el == NULL)
        return;
    // Copy the given event list to the internal list.
    eventList.resize(el->size());
    std::copy(el->begin(), el->end(), eventList.begin());
    scheduleNextWaypoint();
}
