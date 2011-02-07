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

#include "BeaconSender.h"
#include "BeaconSenderMsg_m.h"
#include <NetwControlInfo.h>
#include <SimpleAddress.h>
#include <cassert>

#include "IControllable.h"

Define_Module(BeaconSender);

// ----------------- Public member functions ----------------------------------

void BeaconSender::initialize(int stage)
{
    IDualRadioApp::initialize(stage);
    if(stage == 0) {
        beaconPeriod = par("beaconPeriod");
        beaconTimer = new cMessage( "beacon-timer", BEACON_TIMER );
        scheduleAt(simTime() + beaconPeriod, beaconTimer);
        hpAlwaysOn = par("hpAlwaysOn");
        arrivalTime = simTime();
        discoveredPeer = -1;
        discoveredPeerNetwAddr = -1;
    } else if (stage == 1) {
        if(hpAlwaysOn) {
            ev << "host" << myApplAddr() << " HP configured as always on - turn it on" << endl;
            sendSecondaryXLControl(IControllable::TURN_ON);
            state = WAITING_FOR_HP_ON;
            HPTimer = 0;
        } else {
            HPTimer = new cMessage( "STOP_HP_TIMER", STOP_HP_TIMER );
            // TODO Make this configurable
            hpTimerScale = 3;
            state = IDLE;
        }
    }
}

void BeaconSender::finish()
{
    recordScalar("numFilesDownloaded", downloadedFrom.size());
    recordScalar("sojournTime", simTime() - arrivalTime, "s");
    if (HPTimer != 0) {
        cancelAndDelete(HPTimer);
    }
    cancelAndDelete(beaconTimer);
    IDualRadioApp::finish();
}

// ----------------- Protected member functions -------------------------------

void BeaconSender::handleSelfMsg(cMessage *msg)
{
    if (msg == beaconTimer ) {
        sendBeacon();
        scheduleAt(simTime() + beaconPeriod, beaconTimer);
    } else if (msg == HPTimer) {
        ev << "host" << myApplAddr() <<  " Turning off HP interface" << endl;
        sendSecondaryXLControl(IControllable::TURN_OFF);
        state = IDLE;
    } else {
        throw cRuntimeError("Unknown self-message received");
    }
}

void BeaconSender::handlePrimaryData(cMessage* msg)
{
    switch( msg->getKind() )
    {
    case BEACON_MSG:
    {
        handleBeacon(msg);
        delete msg;
    }
    break;
    default:
        ev << "Received unknown message on primary gate: " << msg->getFullName() << endl;
        delete msg;
        throw cRuntimeError("Unknown message received");
    }
}

void BeaconSender::primaryNicStateUpdate(IControllable::Status newState)
{
    ev << "host" << myApplAddr() << " Primary nic state changed to ";
    switch( newState ){
    case IControllable::TURNED_ON:
        ev<<"TURNED_ON"<<endl;
        break;
    case IControllable::TURNED_OFF:
        ev<<"TURNED_OFF"<<endl;
        break;
    case IControllable::SLEEPING:
        ev<<"SLEEPING"<<endl;
        break;
    default:
        throw cRuntimeError("Unknown state %d received in primaryNicStateUpdate", newState);
    }
}

void BeaconSender::secondaryNicStateUpdate(IControllable::Status newState)
{
    Enter_Method_Silent();
    ev << "host" << myApplAddr() << " Secondary nic state changed to ";
    switch( newState ){
    case IControllable::TURNED_ON:
        ev<<"HP TURNED_ON"<<endl;
        if( state == WAITING_FOR_HP_ON ) {
            state = HP_ON;
            // We need to check if the peer has been destroyed, hence the
            // nodeExists check
            if (discoveredPeer != -1 &&
                    nodeExists(discoveredPeerNetwAddr) &&
                    downloadedFrom.count(discoveredPeer) == 0) {
                assert(discoveredPeerNetwAddr != -1);
                sendFileRequest(discoveredPeer, discoveredPeerNetwAddr);
                discoveredPeer = discoveredPeerNetwAddr = -1;
            }
            if (!hpAlwaysOn) {
                scheduleAt(simTime() + hpTimerScale * beaconPeriod, HPTimer);
            }
        } else {
            throw cRuntimeError("HPNic turned on but state != WAITING_FOR_HP_ON");
        }
        break;
    case IControllable::TURNED_OFF:
        ev<<"TURNED_OFF"<<endl;
        break;
    case IControllable::SLEEPING:
        ev<<"SLEEPING"<<endl;
        break;
    default:
        throw cRuntimeError("Unknown state %d received in secondaryNicStateUpdate", newState);
    }
}


void BeaconSender::handlePrimaryControl(cMessage* msg)
{
    throw cRuntimeError("handlePrimaryControl not implemented");
}

void BeaconSender::handleSecondaryData(cMessage* msg)
{
    switch( msg->getKind() )
    {
    case FILE_REQUEST:
    {
        //NetwControlInfo* nci = check_and_cast<NetwControlInfo*>(msg->getControlInfo());
        //ev << "Network address is " <<nci->getNetwAddr()<<endl;
        ApplPkt *m = static_cast<ApplPkt *>(msg);
        ev << "host" << myApplAddr() << " FILE_REQUEST received from host " << m->getSrcAddr() <<  endl;
        NetwControlInfo* nci = dynamic_cast<NetwControlInfo*>(m->getControlInfo());
        assert(nci != 0);
        sendFileReply(m->getSrcAddr(), nci->getNetwAddr());
    }
    break;
    case FILE_REPLY:
    {
        ApplPkt *m = static_cast<ApplPkt *>(msg);
        ev << "host" << myApplAddr() << " FILE_REPLY received from host " << m->getSrcAddr() <<  endl;
        downloadedFrom.insert(m->getSrcAddr());
        NetwControlInfo* nci = dynamic_cast<NetwControlInfo*>(m->getControlInfo());
        assert(nci != 0);
        sendFileAck(m->getSrcAddr(), nci->getNetwAddr());
    }
    break;
    case FILE_ACK:
    {
        ApplPkt *m = static_cast<ApplPkt *>(msg);
        ev << "host" << myApplAddr() << " FILE_ACK received from host " << m->getSrcAddr() <<  endl;
        uploadedTo.insert(m->getSrcAddr());
    }
    break;
    default:
        ApplPkt *m = static_cast<ApplPkt *>(msg);
        throw cRuntimeError("Unknown message %s received on secondary interface from host %d",
                msg->getFullName(), m->getSrcAddr());
    }
    delete msg;
}

void BeaconSender::handleSecondaryControl(cMessage* msg)
{
    throw cRuntimeError("handleSecondaryControl not implemented");
}

void BeaconSender::sendBeacon()
{
    BeaconSenderMsg *pkt = new BeaconSenderMsg("BEACON_MSG", BEACON_MSG);
    pkt->setDestAddr(-1);
    pkt->setSrcAddr( myApplAddr() );
    pkt->setBitLength(headerLength);
    pkt->setHPNetworkAddress(getSecondaryNetworkAddress());
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    ev << "host" << myApplAddr() << " sending beacon.\n";
    sendPrimaryData(pkt);
}

int BeaconSender::getSecondaryNetworkAddress()
{
    cModule* hpNet = findHost()->getSubmodule("secondaryNet");
    if(!hpNet) {
        throw cRuntimeError("secondaryNet not found");
    }
    // Network address is just the ID of the network module
    return hpNet->getId();
}

void BeaconSender::handleBeacon(cMessage* msg)
{
    BeaconSenderMsg* m = static_cast<BeaconSenderMsg*>(msg);
    int candidateNode = m->getSrcAddr();
    ev << "host" << myApplAddr() <<" Received Beacon from host "<<candidateNode<<endl;
    switch(state)
    {
    case IDLE:
        if (downloadedFrom.count(candidateNode) == 0 || uploadedTo.count(candidateNode) == 0) {
            ev << "host" << myApplAddr() << " Turning on HPnic" << endl;
            sendSecondaryXLControl(IControllable::TURN_ON);
            discoveredPeer = candidateNode;
            discoveredPeerNetwAddr = m->getHPNetworkAddress();
            state = WAITING_FOR_HP_ON;
        } else {
            ev << "host" << myApplAddr() << " Already synchronized with " << candidateNode << endl;
        }
        break;
    case WAITING_FOR_HP_ON:
        ev << "host" << myApplAddr() << " Waiting for HP ON" << endl;
        break;
    case HP_ON:
        if (downloadedFrom.count(candidateNode) == 0 || uploadedTo.count(candidateNode) == 0) {
            updateHPTimer();
        }
        if (downloadedFrom.count(candidateNode) == 0 ) {
            sendFileRequest(candidateNode, m->getHPNetworkAddress());
        }
        break;
    default:
        throw cRuntimeError("Unknown state in BeaconSender");
    }
}

void BeaconSender::updateHPTimer()
{
    if(!hpAlwaysOn) {
        if (!HPTimer->isScheduled()) {
            throw cRuntimeError("state == HP_ON_IDLE but HPTimer is not scheduled");
        }
        cancelEvent(HPTimer);
        scheduleAt(simTime() + hpTimerScale * beaconPeriod, HPTimer);
    }
}

void BeaconSender::sendFileRequest(int dstApplAddress, int dstNetwAddress)
{
    ApplPkt *pkt = new ApplPkt("FILE_REQUEST", FILE_REQUEST);
    pkt->setDestAddr(dstApplAddress);
    pkt->setSrcAddr( myApplAddr() );
    pkt->setBitLength(headerLength);
    pkt->setControlInfo( new NetwControlInfo(dstNetwAddress) );
    ev << "host" << myApplAddr() << " sending FILE_REQUEST to host " << dstApplAddress << endl;
    sendSecondaryData( pkt );
}

void BeaconSender::sendFileReply(int dstApplAddress, int dstNetwAddress)
{
    ApplPkt *pkt = new ApplPkt("FILE_REPLY", FILE_REPLY);
    pkt->setDestAddr(dstApplAddress);
    pkt->setSrcAddr( myApplAddr() );
    pkt->setBitLength(headerLength);
    pkt->setByteLength(1024);
    pkt->setControlInfo( new NetwControlInfo(dstNetwAddress) );
    ev << "host" << myApplAddr() << " sending FILE_REPLY to host " << dstApplAddress << endl;
    updateHPTimer();
    sendSecondaryData( pkt );
}

void BeaconSender::sendFileAck(int dstApplAddress, int dstNetwAddress)
{
    ApplPkt *pkt = new ApplPkt("FILE_ACK", FILE_ACK);
    pkt->setDestAddr(dstApplAddress);
    pkt->setSrcAddr( myApplAddr() );
    pkt->setBitLength(headerLength);
    pkt->setControlInfo( new NetwControlInfo(dstNetwAddress) );
    ev << "host" << myApplAddr() << " sending FILE_ACK to host " << dstApplAddress << endl;
    sendSecondaryData( pkt );
}


bool BeaconSender::nodeExists(int moduleId)
{
    assert(moduleId >= 0);
    return simulation.getModule(moduleId) != 0;
}

/*
void BeaconSender::handleHostState(const HostState& state)
{
    HostState::States hostState = state.get();
      switch (hostState) {
      case HostState::FAILED:
        EV << "t = " << simTime() << " host state FAILED" << endl;
        break;
    default:
        EV << "Unknown host state received" << endl;
    break;
    }
}
*/
