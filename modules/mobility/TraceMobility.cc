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

#include "TraceMobility.h"
#include <FWMath.h>

Define_Module(TraceMobility);

void TraceMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);
    if (stage == 0)
    {
        //hostPtr = getParentModule();
        //hostId = hostPtr->getId();
    	  _currentPos = move.getStartPos();
        _interpolate = par("interpolate").boolValue();
        if (_interpolate)
        {
            _updateInterval = par("updateInterval");
        }
        //_setdestEvent = new cMessage("setdestEvent");
        _updateEvent = new cMessage("updateEvent");
        // Whenever a move-update message and setdest are scheduled at the same time
        // we want to process the move-update first (setdest messages have priority 0)
        // message with smaller priority value is processed first
        _updateEvent->setSchedulingPriority(-1);
        _setdestEvent = 0;
    }
    ev << "TraceMobility initialized at "<<_currentPos.info()<<endl;
}

void TraceMobility::finish()
{
	  BaseMobility::finish();
    cancelAndDelete(_setdestEvent);
    cancelAndDelete(_updateEvent);
}

void TraceMobility::handleSelfMsg(cMessage * msg)
{
    if (msg == _updateEvent)
    {
        makeMove();
        updatePosition();
    }
    else if (msg == _setdestEvent)
    {
        SetDestEv* wp = check_and_cast<SetDestEv*>(_setdestEvent);
        setTarget(wp);
        delete _setdestEvent;
        _setdestEvent = 0;
    }
    else
    {
        BaseMobility::handleMessage(msg);
    }
}

void TraceMobility::makeMove()
{
    _step++;
    if (_step == _numSteps)
    {
        // Destination reached    
        _currentPos = _targetPos;
        move.setSpeed(0);     
        move.setDirectionByVector(Coord(0,0));
        ev << "Target " << _targetPos.info() << " reached, scheduling next waypoint" << endl;
        scheduleNextWaypointEvent();
    }
    else if (_step < _numSteps)
    {
        // step forward
    	  _currentPos = _stepTarget;
        _stepTarget += _stepSize;
        if (_updateEvent->isScheduled())
            cancelEvent(_updateEvent);
        if (_numSteps - _step == 1)
            scheduleAt(_timeAtTarget, _updateEvent);
        else
            scheduleAt(simTime() + _updateInterval, _updateEvent);
    }
    else
    {
        throw cRuntimeError("_step > _numSteps in TraceMobility");
    }
    move.setStart(_currentPos, simTime());
}

void TraceMobility::scheduleNextWaypointEvent()
{
    if (!m_eventList.empty())
    {
        SetDestEv waypoint = m_eventList.front();
        _setdestEvent = new SetDestEv();
        *_setdestEvent = waypoint;
        scheduleAt(_setdestEvent->getTime(), _setdestEvent);
        m_eventList.pop_front();
    }
}

void TraceMobility::setTarget(SetDestEv* waypoint)
{
    simtime_t startTime = waypoint->getTime();
    if (startTime != simTime())
        throw cRuntimeError("Waypoint event is not now");
    _timeAtTarget = waypoint->getTimeAtDest();
    if (simTime() >= _timeAtTarget)
        throw cRuntimeError("timeAtDest is in the past");
   
    // TODO implement 3D support
    _targetPos = Coord(waypoint->getX(), waypoint->getY());
    _step = 0;
    if (_targetPos == _currentPos)
    {
        _numSteps = 0;
        _speed = 0;
        // No need to schedule mobility updates when we are stationary
        move.setSpeed(_speed);
        move.setDirectionByVector(Coord(0,0));
        scheduleNextWaypointEvent();
    }
    else
    {
        if (simTime() == _timeAtTarget)
            throw
                cRuntimeError
                ("simTime() == timeAtDest and destination is not same as current position");

        _speed = _currentPos.distance(_targetPos) / (_timeAtTarget - simTime());
        if( (waypoint->getSpeed() - _speed)*(waypoint->getSpeed() - _speed) > 0.01 ) {
            ev << "waypoint->getSpeed()=" << waypoint->getSpeed() << ", _speed=" << _speed << endl;
            throw cRuntimeError("Speed mismatch in tracefile");
        }
        move.setSpeed(_speed);
        move.setDirectionByTarget(_targetPos);
        // calculate steps for interpolate or non-interpolate
        if (!_interpolate)
        {
            _numSteps = 1;
            if (_updateEvent->isScheduled())
                cancelEvent(_updateEvent);
            scheduleAt(_timeAtTarget, _updateEvent);
        }
        else
        {
            // Get the number of steps needed to be covered.
            simtime_t travelTime = _timeAtTarget - simTime();
            _numSteps = static_cast < int >(ceil(travelTime.dbl() / _updateInterval));
            simtime_t nextEventTime; 
            if (_numSteps > 1)
            {
                _stepSize = (_targetPos - _currentPos) / _numSteps;
                _stepTarget = _currentPos + _stepSize;
                nextEventTime = simTime() + _updateInterval;
            }
            else if (_numSteps == 1)
            {
                nextEventTime = _timeAtTarget;
            }
            else
            {
                throw cRuntimeError("Number of steps less than 0");
            }
            if (_updateEvent->isScheduled())
                cancelEvent(_updateEvent);
            scheduleAt(nextEventTime, _updateEvent);
        }
    }
    ev << "Start moving to " << _targetPos.info() 
        << " in " << _numSteps << " steps "
        << " at speed=" << _speed << endl;
}

void TraceMobility::initializeTrace(const waypointEventsList * eventList)
{
    Enter_Method_Silent();

    if (eventList == NULL)
        return;

    // Copy the given event list to the internal list.
    m_eventList.resize(eventList->size());
    std::copy(eventList->begin(), eventList->end(), m_eventList.begin());
    scheduleNextWaypointEvent();
}
