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

#ifndef MAC80211CONTROL_H_
#define MAC80211CONTROL_H_

#include "Mac80211.h"
#include "IControllable.h"

class Mac80211Control : public Mac80211, public IControllable
{
public:
	enum InternalMacState {IDLE, WAITING_FOR_PHY_ON, WAITING_FOR_PHY_OFF, WAITING_FOR_PHY_SLEEP, WAITING_FOR_PHY_AWAKE};

	virtual void initialize(int);
	//virtual void finish();

protected:
	virtual bool turnOn();
	virtual bool turnOff();

	virtual bool sleep();
	virtual bool wakeUp();
	/**
	 * @brief Overridden to handle IControllable control messages from above.
	 */
	virtual void handleUpperControl(cMessage* msg);

	/**
	 * @brief Overriden to handle IControllable status notifications from
	 * the PHY.
	 */
	virtual void handleLowerControl(cMessage *msg);

	/** @brief Override to silently drop any frames that are received while
	 * we are in off state */
	virtual void handleLowerMsg(cMessage *msg);

	InternalMacState internalState;
};

#endif /* MAC80211CONTROL_H_ */
