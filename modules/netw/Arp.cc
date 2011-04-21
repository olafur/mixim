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

#include "Arp.h"
#include <cassert>
#include <BaseMacLayer.h>
#include "FindModule.h"

Define_Module(Arp);

void Arp::initialize(int stage)
{
    BaseModule::initialize(stage);
}

int Arp::getMacAddr(const int netwAddr)
{
    Enter_Method_Silent();
    cModule* remoteNetwLayer = simulation.getModule( netwAddr );
    if (remoteNetwLayer == 0) {
        return -1;
        //throw cRuntimeError("Module with id %d (i.e. network address) "
        //        "not found in simulation", netwAddr);
    }
    //cModule* remoteHost = remoteNetwLayer->getParentModule();
    cModule* currentLayer = remoteNetwLayer;
    BaseMacLayer* mac = 0;
    // We now have a pointer to the remote network layer module.
    // We traverse 'down' the layers until we find a module which
    // contains a BaseMacLayer and assume that the containing module
    // is a nic.
    do {
        ev << "Current layer is " << currentLayer->getName() << endl;
        if(currentLayer->hasGate("lowerGateOut")) {
            cGate* currentLayerOut = currentLayer->gate("lowerGateOut");
            cGate* lowerLayerIn = currentLayerOut->getNextGate();
            assert(lowerLayerIn != 0);
            cModule* lowerLayer = lowerLayerIn->getOwnerModule();
            //mac = dynamic_cast<BaseMacLayer*>(lowerLayer);
            currentLayer = lowerLayer;
        } else {
            // No 'lowerGateOut', we assume that currentLayer is a nic
            mac = FindModule<BaseMacLayer*>::findSubModule(currentLayer);
            if (mac == 0) {
                throw cRuntimeError("No 'BaseMacLayer' found at the remote node");
            }
            ev << "Mac layer found: " << mac->getName() << endl;
            // TODO This breaks if an addressing scheme is used.  BaseMacLayer should
            // have a public method to obtain the address (but this is currently not
            // implemented in MiXiM)
            return currentLayer->getId();
        }
    } while(true);
    throw cRuntimeError("Unreachable code");
}
