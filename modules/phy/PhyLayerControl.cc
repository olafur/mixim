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

#include "PhyLayerControl.h"
#include "NicController.h"

Define_Module(PhyLayerControl);

// ----------------- Public member functions ----------------------------------

void PhyLayerControl::initialize(int stage) {

    if(stage == 0) {
        initialRadioState = par("initialRadioState").longValue();
    }

    bool startsOn = getParentModule()->par("startsOn").boolValue();
    if (startsOn) {
        PhyLayerBattery::initialize(stage);
        status = IControllable::TURNED_ON;
        //on = true;
    } else {
        // The only initialization that we need to defer is the setting of the
        // radio current (which initializes the current draw from the battery).
        // This is done in stage 2 in the parent class
        status = IControllable::TURNED_OFF;
        //on = false;
        if (stage == 0) {
            PhyLayerBattery::initialize(stage);
        } else if (stage == 1) {
            PhyLayer::initialize(stage);
            registerWithBattery("physical layer", numActivities);
        }
    }
}

void PhyLayerControl::receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
    BatteryAccess::receiveBBItem(category, details, scopeModuleId);
    if(category == catMove)
    {
        Move m(*static_cast<const Move*>(details));
        // We only update the position of the node in the ConnectionManager
        // if the interface is turned on.  When the nic is turned off it is
        // registered in the ConnectionManager and invisible to other nic's.

        if (isOn()) {
            if(isRegistered) {
                cc->updateNicPos(getParentModule()->getId(), &m.getStartPos());
            } else {
                // register the nic with ConnectionManager
                // returns true, if sendDirect is used
                useSendDirect = cc->registerNic(getParentModule(), this, &m.getStartPos());
                isRegistered = true;
            }
        }
        move = m;
        coreEV<<"new HostMove: "<<move.info()<<endl;
    }
}

// ----------------- Protected member functions -------------------------------

bool PhyLayerControl::turnOn()
{
    ev << "Turning PHY on" << endl;
    // Start drawing current according to the 'initialRadioState'
    simtime_t switchtime = setRadioState(initialRadioState);
    if(switchtime == -1) {
        // TODO This can occur if switching times between radio states are not 0
        // and the radio is busy switching to some other state.  For now we just
        // throw an exception - need to consider this scenario.
        throw cRuntimeError("Could not set radio state");
    }
    // Register the nic with the channel control
    useSendDirect = cc->registerNic(getParentModule(), this, &move.getStartPos());
    isRegistered = true;
    ev << "PHY turned on" << endl;
    return true;
}

bool PhyLayerControl::turnOff()
{
    ev << "Turning PHY off" << endl;
    if (!cc->unregisterNic(getParentModule())) {
        throw cRuntimeError("Could not unregister NIC");
    }
    decider->finish();
    isRegistered = false;

    if(txOverTimer) {
        cancelEvent(txOverTimer);
    }
    AirFrameVector channel;
    channelInfo.getAirFrames(0, simTime(), channel);
    for(AirFrameVector::iterator it = channel.begin();
            it != channel.end(); ++it)
    {
        cancelAndDelete(*it);
    }
    channelInfo = ChannelInfo();

    // TODO Create state Radio::OFF
    // Since no OFF state exists in mixim, we use the the SLEEP state
    // (and set the current accordingly)
    simtime_t switchtime = setRadioState(Radio::SLEEP);
    if(switchtime == -1) {
        // TODO This can occur if switching times between radio states are not 0
        // and the radio is busy switching to some other state.  For now we just
        // throw an exception - need to consider this scenario.
        throw cRuntimeError("Could not set radio state");
    }

    ev << "PHY turned off" << endl;
    return true;
}


void PhyLayerControl::handleUpperControlMessage(cMessage* msg)
{
    ev<<"PhyLayerControl received upper control "<<msg->getName()<<endl;
    switch(msg->getKind()) {
    case IControllable::TURN_ON:
        if(!isOn() && !isSleeping()) {
            if (!turnOn()) {
                throw cRuntimeError("Could not TURN_ON the PHY");
            }
        } else {
            if (!wakeUp()) {
                throw cRuntimeError("Could not WAKEUP the PHY");
            }
        }
        status = IControllable::TURNED_ON;
        sendControlUp(new cMessage("TURNED_ON", IControllable::TURNED_ON));
        delete msg;
        break;
    case IControllable::TURN_OFF:
        if(isOn() || isSleeping()) {
            if(!turnOff()) {
                throw cRuntimeError("Could not TURN_OFF the PHY");
            }
        }
        status = IControllable::TURNED_OFF;
        sendControlUp(new cMessage("TURNED_OFF", IControllable::TURNED_OFF));
        delete msg;
        break;
    case IControllable::SLEEP:
        if (isSleeping()) {
            throw cRuntimeError("Trying to send PhyLayer to SLEEP when it is already SLEEPING");
        }
        if (!sleep()) {
            throw cRuntimeError("Could not put PHY to SLEEP");
        }
        status = IControllable::SLEEPING;
        sendControlUp(new cMessage("SLEEPING", IControllable::SLEEPING));
        delete msg;
        break;
    default:
        PhyLayerBattery::handleUpperControlMessage(msg);
        //throw cRuntimeError("Received unknown control message from upper layer!");
    }

}

bool PhyLayerControl::sleep()
{
    return true;
    //TODO
}

bool PhyLayerControl::wakeUp()
{
    return true;
    //TODO
}

void PhyLayerControl::sendControlUp(cMessage *msg) {
    send(msg, upperControlOut);
}


