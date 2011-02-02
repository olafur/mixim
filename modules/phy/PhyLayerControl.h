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

#ifndef PHYLAYERCONTROL_H_
#define PHYLAYERCONTROL_H_

#include "PhyLayerBattery.h"
#include "BaseBattery.h"
#include "IControllable.h"

class PhyLayerControl : public PhyLayerBattery, public IControllable {
public:
	virtual void initialize(int stage);
	//virtual void finish();

	/**
	 * Overrides how position updates are handled.  In particular we need
	 * to handle position updates differently from what is done in
	 * ChannelAccess.
	 */
	virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:
	/**
	 * @brief Handles messages received from the upper layer through the
	 * control gate.
	 */
	virtual void handleUpperControlMessage(cMessage* msg);

	// Overridden functions from IControllable
	virtual bool turnOn();
	virtual bool turnOff();
	virtual bool sleep();
	virtual bool wakeUp();

	int initialRadioState;

	void sendControlUp(cMessage * msg);
};

#endif /* PHYLAYERCONTROL_H_ */
