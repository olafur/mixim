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

#include "NicController.h"
#include <omnetpp.h>
#include "NicCommandNotify.h"
#include <cassert>

Define_Module(NicController);

// ----------------- Public member functions ----------------------------------

void NicController::initialize(int stage) {
    BaseLayer::initialize(stage);
    if (stage == 0) {
        startsOn = getParentModule()->par("startsOn").boolValue();
        if(startsOn) {
            status = TURNED_ON;
        } else {
            // TODO Currently we assume that a nic can only be initialized
            // in 'on' or 'off' mode (i.e. not 'sleep').  We may want to
            // fix this at some point.
            status = TURNED_OFF;
        }
        cModule* host = findHost();
        if (host == 0) {
            throw cRuntimeError("Hostpointer is NULL!");
        }
        hostId = host->getId();

        delayOffToOn = par("delayOffToOn").doubleValue();
        delaySleepToOn = par("delaySleepToOn").doubleValue();
        if (delayOffToOn > 0 || delaySleepToOn > 0) {
            turnOnTimer = new cMessage("turnOnTimer");
        } else {
            turnOnTimer = 0;
        }
        NicCommandNotify stateChangeCommand;
        catStateChangeCommands = utility->subscribe(this, &stateChangeCommand, hostId);
        stateChangeInfo.nicId = this->getNicId();
        catStateChangeInfo = utility->getCategory(&stateChangeInfo);
        internalState = NicController::NONE;
    }
    if (stage == 1) {
        stateChangeInfo.status = status;
        utility->publishBBItem(catStateChangeInfo, &stateChangeInfo, hostId);
    }
}


void NicController::finish()
{
    cancelAndDelete(turnOnTimer);
    utility->unsubscribe(this, catStateChangeCommands);
    BaseLayer::finish();
}

bool NicController::turnOn()
{
    if(status == IControllable::TURNED_ON ) {
        throw cRuntimeError("Trying to turn on a NIC that is already ON");
    }
    if(status == IControllable::SLEEPING ) {
        throw cRuntimeError("Trying to turn on a NIC that is SLEEPING (use WAKE_UP)");
    }
    if (internalState != NicController::NONE) {
        // TODO Currently we just throw an error if we receive a turn on
        // and the internal state is not NONE.  Implement this.
        throw cRuntimeError("Trying to turn on a nic while internal "
                "state not NONE (internalState = %d)", internalState);
    }
    assert(status == IControllable::TURNED_OFF);
    if (delayOffToOn > 0) {
        assert(turnOnTimer && !turnOnTimer->isScheduled());
        scheduleAt(simTime() + delayOffToOn, turnOnTimer);
        internalState = NicController::WAITING_FOR_OFFTOON_TIMER;
    } else {
        internalState = NicController::WAITING_FOR_ON;
        sendControlDown(new cMessage("TURN_ON", IControllable::TURN_ON));
    }
    return true;
}

bool NicController::sleep()
{
    if(status == IControllable::SLEEPING) {
        throw cRuntimeError("Trying to suspend a nic that is already SLEEPING");
    }
    if (turnOnTimer && turnOnTimer->isScheduled()) {
        cancelEvent(turnOnTimer);
    }
    internalState = NicController::WAITING_FOR_SLEEP;
    status = IControllable::SLEEPING;
    // When going to sleep or turning off, we change the state immediately and
    // publish on the blackboard so that applications do not send us any data
    // while we are shutting down
    sendControlDown(new cMessage("SLEEP", IControllable::SLEEP));
    publishNicState();
    return true;
}

bool NicController::wakeUp()
{
    if(status == IControllable::TURNED_ON ) {
        throw cRuntimeError("Trying to WAKE_UP a NIC that is already ON");
    }
    if(status == IControllable::TURNED_OFF) {
        throw cRuntimeError("Trying to WAKE_UP a NIC that OFF (use TURN_ON)");
    }
    if (internalState != NicController::NONE) {
        // TODO Currently we just throw an error if we receive a wakeup
        // and the internal state is not NONE.  Implement this.
        throw cRuntimeError("Trying to wake up a nic while internal "
                "state not NONE (internalState = %d)", internalState);
    }
    assert(status == IControllable::SLEEPING);
    if ( delaySleepToOn > 0) {
        assert(turnOnTimer && !turnOnTimer->isScheduled());
        scheduleAt(simTime() + delaySleepToOn, turnOnTimer);
        internalState = NicController::WAITING_FOR_SLEEPTOON_TIMER;
    } else {
        internalState = NicController::WAITING_FOR_ON;
        sendControlDown(new cMessage("WAKE_UP", IControllable::WAKE_UP));
    }
    return true;
}

bool NicController::turnOff()
{
    if(status == IControllable::TURNED_OFF) {
        throw cRuntimeError("Trying to turn off a nic that is already TURNED_OFF");
    }
    if (turnOnTimer && turnOnTimer->isScheduled()) {
        cancelEvent(turnOnTimer);
    }
    internalState = NicController::WAITING_FOR_OFF;
    status = IControllable::TURNED_OFF;
    // When going to sleep or turning off, we change the state immediately and
    // publish on the blackboard so that applications do not send us any data
    // while we are shutting down
    sendControlDown(new cMessage("TURN_OFF", IControllable::TURN_OFF));
    publishNicState();
    return true;
}

void NicController::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    Enter_Method_Silent();
    if (category == catStateChangeCommands) {
        NicCommandNotify* notify = check_and_cast<NicCommandNotify*>(details);
        if (notify->nicId == getNicId()) {
            handleXLUpperControl(notify->command);
        }
    }
    BaseModule::receiveBBItem(category, details, scopeModuleId);
}


// ----------------- Protected member functions -------------------------------

void NicController::handleSelfMsg(cMessage* msg)
{
    if (msg != this->turnOnTimer) {
        throw cRuntimeError("unknown message (%s) received by NicController",
                msg->getFullName());
    }
    if (internalState == NicController::WAITING_FOR_OFFTOON_TIMER ||
            internalState == NicController::WAITING_FOR_SLEEPTOON_TIMER ) {
        internalState = NicController::WAITING_FOR_ON;
        sendControlDown(new cMessage("TURN_ON", IControllable::TURN_ON));
    } else {
        throw cRuntimeError("State error: turnOnTimer expired in internalState=%d, "
                "status=%d", internalState, this->status );
    }
}

/* Handle cross-layer messages */
void NicController::handleXLUpperControl(IControllable::Controls command)
{
    switch(command)
    {
        case IControllable::TURN_ON:
            turnOn();
            break;
        case IControllable::TURN_OFF:
            turnOff();
            break;
        case IControllable::SLEEP:
            sleep();
            break;
        case IControllable::WAKE_UP:
            wakeUp();
            break;
        default:
            throw cRuntimeError("Received unknown control message");
    }
}

/** Simple redirection of data messages */
void NicController::handleUpperMsg(cMessage* msg) 
{
    if (isOn()) {
        sendDown(msg);
    } else {
        throw cRuntimeError("Trying to send a message down but NIC is not ON");
    }
}

/** Simple redirection of data messages */
void NicController::handleLowerMsg(cMessage* msg) 
{
    if (isOn()) {
        sendUp(msg);
    } else {
        throw cRuntimeError("Trying to send a message up but NIC is not ON");
    }
}

void NicController::handleLowerControl(cMessage* msg)
{
    switch(msg->getKind()) {
    case IControllable::TURNED_ON:
        ev << "NicController is ON" << endl;
		if (internalState != NicController::WAITING_FOR_ON) {
			delete msg;
		    throw cRuntimeError("Received TURNED_ON from MAC but not WAITING_FOR_ON");
		}
		status = IControllable::TURNED_ON;
		internalState = NicController::NONE;
		publishNicState();
		delete msg;
		break;
	case IControllable::TURNED_OFF:
		ev << "NicController is OFF" << endl;
		if (internalState != NicController::WAITING_FOR_OFF) {
			delete msg;
		    throw cRuntimeError("Received TURNED_OFF from MAC but not WAITING_FOR_OFF");
		}
		status = IControllable::TURNED_OFF;
		internalState = NicController::NONE;
		// State change was published when the control was received from above and therefore
		// we do not have to publish the state now.
		delete msg;
		break;
	case IControllable::SLEEPING:
		ev << "NicController is SLEEPING" << endl;
		if (internalState != NicController::WAITING_FOR_SLEEP) {
			delete msg;
		    throw cRuntimeError("Received SLEEPING from MAC but not WAITING_FOR_SLEEP");
		}
		status = IControllable::SLEEPING;
		internalState = NicController::NONE;
        // State change was published when the control was received from above and therefore
        // we do not have to publish the state now.
		delete msg;
		break;
	default:
		//delete msg;
	    //throw cRuntimeError("Received unknown control message from lower layer!");
	    this->sendControlUp(msg);
		break;
	}
}

void NicController::handleUpperControl(cMessage* msg)
{
	throw cRuntimeError("NicController does not handle upper control messages");
}

// ----------------- Private member functions ---------------------------------

void NicController::publishNicState()
{
    stateChangeInfo.status = status;
    utility->publishBBItem(catStateChangeInfo, &stateChangeInfo, hostId);
}
