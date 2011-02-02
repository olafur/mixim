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

#include "IDualRadioApp.h"
#include "NicController.h"

// ----------------- Public member functions ----------------------------------

void IDualRadioApp::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage == 0) {
        primaryDataIn = findGate("primaryDataIn");
        primaryDataOut = findGate("primaryDataOut");
        primaryControlIn = findGate("primaryControlIn");
        primaryControlOut = findGate("primaryControlOut");

        secondaryDataIn = findGate("secondaryDataIn");
        secondaryDataOut = findGate("secondaryDataOut");
        secondaryControlIn = findGate("secondaryControlIn");
        secondaryControlOut = findGate("secondaryControlOut");

        headerLength= par("headerLength");

        // Find the IDs of the nics.
        cModule* host = findHost();
        if(host == 0) {
            throw cRuntimeError("Host pointer is NULL in IDualRadioApp");
        }
        cModule* primaryNic = host->getSubmodule("primaryNic");
        if(primaryNic == 0) {
            throw cRuntimeError("Could not find module 'primaryNic' in IDualRadioApp");
        }
        cModule* nicController = primaryNic->getSubmodule("nicController");
        if (nicController != 0) {
            NicController* nicCtrl = check_and_cast<NicController*>(nicController);
            if(nicController == 0) {
                throw cRuntimeError("Could not find module primary 'nicController' in IDualRadioApp");
            }
            primaryNicId = nicCtrl->getNicId();
        } else {
            primaryNicId = -1;
        }

        cModule* secondaryNic = host->getSubmodule("secondaryNic");
        if(secondaryNic == 0) {
            throw cRuntimeError("Could not find module 'secondaryNic' in IDualRadioApp");
        }
        nicController = secondaryNic->getSubmodule("nicController");
        if(nicController != 0) {
            NicController* nicCtrl = check_and_cast<NicController*>(nicController);
            if(nicController == 0) {
                throw cRuntimeError("Could not find module secondary 'nicController' in IDualRadioApp");
            }
            secondaryNicId = nicCtrl->getNicId();
        } else {
            secondaryNicId = -1;
        }

        // Subscribe to nic state changes.
        NicStateNotify stateChangeInfo;
        catNicStateNotify = utility->subscribe(this, &stateChangeInfo, host->getId());

        // Obtain category for publishing nic commands on
        NicCommandNotify nicCommandNotify;
        catNicCommandNotify = utility->getCategory(&nicCommandNotify);
    }
}

void IDualRadioApp::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()){
        handleSelfMsg(msg);
    } else if(msg->getArrivalGateId() == primaryDataIn) {
        handlePrimaryData(msg);
    } else if(msg->getArrivalGateId() == primaryControlIn) {
        handlePrimaryControl(msg);
    } else if(msg->getArrivalGateId() == secondaryDataIn) {
        handleSecondaryData(msg);
    } else if(msg->getArrivalGateId() == secondaryControlIn) {
        handleSecondaryControl(msg);
    } else {
        throw cRuntimeError("Received message on unknown gate %d", msg->getArrivalGateId());
    }
}

void IDualRadioApp::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    if (category == catNicStateNotify) {
        NicStateNotify* notify = check_and_cast<NicStateNotify*>(details);
        if (primaryNicId != -1 && notify->nicId == primaryNicId) {
            primaryNicStateUpdate(notify->status);
        } else if (secondaryNicId != -1 && notify->nicId == secondaryNicId) {
            secondaryNicStateUpdate(notify->status);
        } else {
            throw cRuntimeError("NIC status notify on unknown nicId %d", notify->nicId);
        }
    }
    BaseModule::receiveBBItem(category, details, scopeModuleId);
}

// ----------------- Protected member functions -------------------------------

void IDualRadioApp::sendPrimaryData(cMessage *msg)
{
    send(msg, primaryDataOut);
}

void IDualRadioApp::sendPrimaryControl(cMessage *msg)
{
    send(msg, primaryControlOut);
}

void IDualRadioApp::sendSecondaryData(cMessage *msg)
{
    send(msg, secondaryDataOut);
}

void IDualRadioApp::sendSecondaryControl(cMessage *msg)
{
    send(msg, secondaryControlOut);
}

void IDualRadioApp::sendPrimaryXLControl(IControllable::Controls command)
{
    if (primaryNicId == -1) {
        throw cRuntimeError("Trying to control primary nic but it is not controllable");
    }
    NicCommandNotify nicCommandNotify;
    nicCommandNotify.command = command;
    nicCommandNotify.nicId = primaryNicId;
    utility->publishBBItem(catNicCommandNotify, &nicCommandNotify, findHost()->getId());
}

void IDualRadioApp::sendSecondaryXLControl(IControllable::Controls command)
{
    if (secondaryNicId == -1) {
        throw cRuntimeError("Trying to control secondary nic but it is not controllable");
    }
    NicCommandNotify nicCommandNotify;
    nicCommandNotify.command = command;
    nicCommandNotify.nicId = secondaryNicId;
    utility->publishBBItem(catNicCommandNotify, &nicCommandNotify, findHost()->getId());
}
