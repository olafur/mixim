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

#ifndef NICCONTROLLER_H_
#define NICCONTROLLER_H_

#include <BaseLayer.h>
#include "IControllable.h"
#include "NicStateNotify.h"

class NicController : public BaseLayer, IControllable
{
public:
    virtual void initialize(int stage);
    virtual void finish();

    /** @brief Returns a unique id for the nic*/
    int getNicId() const {return this->getId();}

    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:
    /** @brief Not needed.  Throws an exception if ever called. */
    virtual void handleSelfMsg(cMessage* msg);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage *msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

    /** @brief Handle control messages from upper layer */
    virtual void handleUpperControl(cMessage *msg);

    /** @brief Handle control messages received on the blackboard */
    virtual void handleXLUpperControl(IControllable::Controls command);
    
    // Overridden functions from IControllable
    virtual bool turnOn();
    virtual bool sleep();
    virtual bool wakeUp();
    virtual bool turnOff();

private:
    enum InternalState {NONE, WAITING_FOR_ON, WAITING_FOR_OFFTOON_TIMER, WAITING_FOR_SLEEPTOON_TIMER, WAITING_FOR_OFF, WAITING_FOR_SLEEP};
    InternalState internalState;

    /** @brief Should the NIC be turned on when created?*/
    bool startsOn;

    /** @brief The state of the nic to be published on the blackboard and
    * its blackboard category.*/
    NicStateNotify stateChangeInfo;
    int catStateChangeInfo;

    /** @brief The category for control messages sent to the controller. */
    int catStateChangeCommands;

    int hostId;
    simtime_t delayOffToOn, delaySleepToOn;

    cMessage* turnOnTimer;

    void publishNicState();
};

#endif /* NICCONTROLLER_H_ */
