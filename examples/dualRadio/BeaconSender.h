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

#ifndef BEACON_SENDER_H
#define BEACON_SENDER_H

#include "IDualRadioApp.h"
#include <set>
//#include <HostState.h>

using namespace std;

class BeaconSender : public IDualRadioApp
{
public:
    virtual void initialize(int);
    virtual void finish();
    //virtual void handleHostState(const HostState& state);

    enum BeaconSenderEvents {BEACON_TIMER, STOP_HP_TIMER};
    enum BeaconSenderMsgKinds {BEACON_MSG, FILE_REQUEST, FILE_REPLY, FILE_ACK};


protected:
    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage* msg);

    /**
     * Handler routines for incoming messages
     */
    virtual void handlePrimaryData(cMessage* msg);
    virtual void handlePrimaryControl(cMessage* msg);
    virtual void handleSecondaryData(cMessage* msg);
    virtual void handleSecondaryControl(cMessage* msg);

    /** @brief Called when the state of the primary NIC changes */
    virtual void primaryNicStateUpdate(IControllable::Status newState);
    /** @brief Called when the state of the secondary NIC changes */
    virtual void secondaryNicStateUpdate(IControllable::Status newState);

    void handleBeacon(cMessage* msg);
    void handleFileRequest();

    void sendBeacon();
    void sendFileRequest(int dstApplAddress, int dstNetwAddress);
    void sendFileReply(int dstApplAddress, int dstNetwAddress);
    void sendFileAck(int dstApplAddress, int dstNetwAddress);

private:
    enum BeaconSenderStates {IDLE, WAITING_FOR_HP_ON, HP_ON};
    BeaconSenderStates state;
    set<int> downloadedFrom, uploadedTo;
    int discoveredPeer;
    int discoveredPeerNetwAddr;
    bool hpAlwaysOn;

    int getSecondaryNetworkAddress();
    void updateHPTimer();
    bool nodeExists(int moduleId);

    //IControllable::Status hpState;
    double beaconPeriod;
    cMessage* beaconTimer;
    cMessage* HPTimer;
    int hpTimerScale;

    simtime_t arrivalTime;
};

#endif

