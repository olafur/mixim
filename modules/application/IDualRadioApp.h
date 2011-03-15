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

#ifndef IDUALRADIOAPP_H_
#define IDUALRADIOAPP_H_

#include <BaseModule.h>
#include <TraceMobility.h>
#include "IControllable.h"
#include "NicCommandNotify.h"

class IDualRadioApp : public BaseModule {
public:

	virtual ~IDualRadioApp() {};

	virtual void initialize(int);
	//virtual void finish();

	/**
	 * The basic handle message function.
	 *
	 * Depending on the gate a message arrives handleMessage just calls
	 * different handle*Msg functions to further process the message.
	 *
	 * You should not make any changes in this function but implement all
	 * your functionality into the handle*Msg functions called from here.
	 *
	 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
	 **/
    virtual void handleMessage(cMessage* msg);

    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage* msg) = 0;

    /**
     * Handler routines for incoming messages
     */
    virtual void handlePrimaryData(cMessage* msg) = 0;
    virtual void handlePrimaryControl(cMessage* msg) = 0;
    virtual void handleSecondaryData(cMessage* msg) = 0;
	virtual void handleSecondaryControl(cMessage* msg) = 0;

	/** @brief Called when the state of the primary NIC changes */
	virtual void primaryNicStateUpdate(IControllable::Status newState) = 0;
	/** @brief Called when the state of the secondary NIC changes */
	virtual void secondaryNicStateUpdate(IControllable::Status newState) = 0;

	/**
	 * Convenience functions for sending to out-gates.
	 */
	void sendPrimaryData(cMessage *msg);
	void sendPrimaryControl(cMessage *msg);
	void sendSecondaryData(cMessage *msg);
	void sendSecondaryControl(cMessage *msg);

	virtual void sendPrimaryXLControl(IControllable::Controls command);
	virtual void sendSecondaryXLControl(IControllable::Controls command);

	/**
	 * @brief Return my application layer address
	 *
	 * If the node has a mobility module of type 'TraceMobility' we use the
	 * nodeId from the mobility trace file.  Otherwise it returns the index
	 * of the parent module
	 **/
	virtual const int myApplAddr() {
	    cModule* parent = getParentModule();
	    cModule* mobility = parent->getSubmodule("mobility");
	    if(mobility) {
	        TraceMobility *tm = dynamic_cast<TraceMobility *>(mobility);
	        if(tm != 0) {
	           return tm->getNodeId();
	        }
	    }
	    return parent->getIndex();
	};

    int primaryDataIn, primaryDataOut;
	int primaryControlIn, primaryControlOut;

    int secondaryDataIn, secondaryDataOut;
    int secondaryControlIn, secondaryControlOut;

    int primaryNicId, secondaryNicId;

    /** @brief Blackboard category for state update notifications */
    int catNicStateNotify;

    /** @brief Blackboard category for sending commands to nic's*/
    int catNicCommandNotify;

    int headerLength;
};

#endif /* IDUALRADIOAPP_H_ */
