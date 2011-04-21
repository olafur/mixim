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

#include "Mac80211Control.h"
#include <Decider80211.h>

Define_Module(Mac80211Control);

// ----------------- Public member functions ----------------------------------

void Mac80211Control::initialize(int stage) {
	bool startsOn = getParentModule()->par("startsOn").boolValue();
	if(startsOn) {
	    status = IControllable::TURNED_ON;
		Mac80211::initialize(stage);
	} else {
	    status = IControllable::TURNED_OFF;
		if (stage == 0) {
			Mac80211::initialize(stage);
		} else if (stage == 1) {
			// We can do all the same initialization as in the parent 802.11 module
			// apart from the last step.  We do not want to start sensing the
			// channel until the nic has been turned on.
			BaseMacLayer::initialize(stage);
			BaseConnectionManager* cc = getConnectionManager();
			if(cc->hasPar("pMax") && txPower > cc->par("pMax").doubleValue())
				opp_error("TranmitterPower can't be bigger than pMax in ConnectionManager! "
						  "Please adjust your omnetpp.ini file accordingly.");

			//TODO: save channel to radio or as member variable!
			int channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;
			bool found = false;
			bitrate = hasPar("bitrate") ? par("bitrate").doubleValue() : BITRATES_80211[0];
			for(int i = 0; i < 4; i++) {
				if(bitrate == BITRATES_80211[i]) {
					found = true;
					break;
				}
			}
			if(!found) bitrate = BITRATES_80211[0];
			defaultBitrate = bitrate;

			snrThresholds.push_back(hasPar("snr2Mbit") ? par("snr2Mbit").doubleValue() : 100);
			snrThresholds.push_back(hasPar("snr5Mbit") ? par("snr5Mbit").doubleValue() : 100);
			snrThresholds.push_back(hasPar("snr11Mbit") ? par("snr11Mbit").doubleValue() : 100);
			snrThresholds.push_back(111111111); // sentinel

			neighborhoodCacheSize = hasPar("neighborhoodCacheSize") ? par("neighborhoodCacheSize").longValue() : 0;
			neighborhoodCacheMaxAge = hasPar("neighborhoodCacheMaxAge") ? par("neighborhoodCacheMaxAge").longValue() : 10000;

			EV << " MAC Address: " << myMacAddr
			   << " rtsCtsThreshold: " << rtsCtsThreshold
			   << " bitrate: " << bitrate
			   << " channel: " << channel
			   << " autoBitrate: " << autoBitrate
			   << " 2MBit: " << snrThresholds[0]
			   << " 5.5MBit: " <<snrThresholds[1]
			   << " 11MBit: " << snrThresholds[2]
			   << " neighborhoodCacheSize " << neighborhoodCacheSize
			   << " neighborhoodCacheMaxAge " << neighborhoodCacheMaxAge
			   << endl;

			for(int i = 0; i < 3; i++) {
				snrThresholds[i] = FWMath::dBm2mW(snrThresholds[i]);
			}
		}
	}
}

// ----------------- Protected member functions -------------------------------

bool Mac80211Control::turnOn()
{

    if (isOn()) {
        throw cRuntimeError("Trying to turn ON interface while already ON");
        return false;
    }
    if (internalState == WAITING_FOR_PHY_ON) {
        throw cRuntimeError("Received TURN_ON while waiting for phy to become ready");
        return false;
    }
    if (contention == 0) {
        contention = new ChannelSenseRequest("contention", MacToPhyInterface::CHANNEL_SENSE_REQUEST);
        contention->setSenseMode(UNTIL_BUSY);
    }
    if(endSifs == 0){
        endSifs = new ChannelSenseRequest("end SIFS", MacToPhyInterface::CHANNEL_SENSE_REQUEST);
        endSifs->setSenseMode(UNTIL_BUSY);
        endSifs->setSenseTimeout(SIFS);
        endSifs->setContextPointer(0);
    }
	remainingBackoff = backoff();
	senseChannelWhileIdle(remainingBackoff + currentIFS);
	status = IControllable::TURNED_ON;
	ev << "Mac80211Control is now TURNED_ON" << endl;
	return true;
}

bool Mac80211Control::turnOff()
{
    if(status == IControllable::TURNED_OFF) {
        throw cRuntimeError("Trying to turn OFF while already OFF");
        return false;
    }

	// Cancel all timers
	cancelEvent(timeout);
	cancelEvent(nav);
	if(endSifs) {
	    // Data frames, RTS and CTS frames are stored in the context pointer
	    // and deleted when an ACK arrives (or timeout).  We need to delete
	    // these stored frames here or else they are never freed up.
	    if (endSifs->getContextPointer()) {
	        delete static_cast<Mac80211Pkt *>(endSifs->getContextPointer());
	        endSifs->setContextPointer(0);
	    }
	    cancelEvent(endSifs);
	    delete endSifs;
	    endSifs = 0;
	}
	if(contention) {
	    cancelEvent(contention);
        delete contention;
        contention = 0;
    }
	// Silently drop and delete all packets from above
	MacPktList::iterator it;
	for(it = fromUpperLayer.begin(); it != fromUpperLayer.end(); ++it) {
        delete (*it);
    }
    fromUpperLayer.clear();
    // Reset state variables
    state = Mac80211::IDLE;
    longRetryCounter = 0;
    shortRetryCounter = 0;
    currentIFS = EIFS;
    status = IControllable::TURNED_OFF;
    ev << "Mac80211Control is now TURNED_OFF" << endl;
    return true;
}

void Mac80211Control::handleUpperControl(cMessage* msg) {
	ev<<"Mac80211Control received upper control "<<msg->getName()<<endl;
    switch(msg->getKind()) {
	case (IControllable::TURN_ON):
	{
	    internalState = WAITING_FOR_PHY_ON;
		sendControlDown(msg);
	}
	break;
	case (IControllable::TURN_OFF):
	{
		internalState = WAITING_FOR_PHY_OFF;
		sendControlDown(msg);
	}
	break;
	case (IControllable::SLEEP):
	{
	    if (sleep() == false) {
	        throw cRuntimeError("Could not send MAC80211 to sleep");
	    }
		internalState = WAITING_FOR_PHY_SLEEP;
		sendControlDown(msg);
	}
	break;
	case (IControllable::WAKE_UP):
    {
        if (wakeUp() == false) {
            throw cRuntimeError("Could not wake up MAC80211");
        }
        internalState = WAITING_FOR_PHY_AWAKE;
        sendControlDown(msg);
    }
    break;
	default:
		throw cRuntimeError("Received unknown control message from upper layer!");
	}
}

void Mac80211Control::handleLowerControl(cMessage *msg) {
    ev<<"Mac80211Control received lower control "<<msg->getName()<<endl;
	switch(msg->getKind()) {
	case (IControllable::TURNED_ON):
	{
	    if (internalState != WAITING_FOR_PHY_ON) {
	        throw cRuntimeError("Received TURNED_ON from PHY but not WAITING_FOR_PHY_ON");
	    }
	    internalState = IDLE;
	    if(turnOn() == false) {
            throw cRuntimeError("Could not turn on MAC80211");
        }
	    sendControlUp(msg);
	}
	break;
	case (IControllable::TURNED_OFF):
	{
	    if (internalState != WAITING_FOR_PHY_OFF) {
	        throw cRuntimeError("Received TURNED_OFF from PHY but not WAITING_FOR_PHY_OFF");
        }
	    if(turnOff() == false) {
	        throw cRuntimeError("Could not turn off MAC80211");
        }
	    internalState = IDLE;
		sendControlUp(msg);
	}
	break;
	// TODO: Implement sleep/wakeup
	default:
	{
	    if (status != IControllable::TURNED_ON) {
	        if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
	            // The PHY sends a RADIO_SWITCHING_OVER message up to the mac
	            // during its initialization.
	            Mac80211::handleLowerControl(msg);
	        } else if (msg->getKind() == Decider80211::BITERROR) {
	            Mac80211::handleLowerControl(msg);
	        } else {
	            throw cRuntimeError("Received control %s from PHY while not ON", msg->getFullName());
	        }
	    } else {
	        Mac80211::handleLowerControl(msg);
	    }
	}
	break;
	}
}

void Mac80211Control::handleLowerMsg(cMessage *msg)
{
    if (status != IControllable::TURNED_ON) {
        throw cRuntimeError("Received message %s from PHY while not ON", msg->getFullName());
    }
    Mac80211::handleLowerMsg(msg);
}

bool Mac80211Control::sleep()
{
    ev << "Mac80211Control Received a SLEEP";
    // TODO Implement!
    throw cRuntimeError("sleep not yet implemented");
	return true;
}

bool Mac80211Control::wakeUp()
{
    ev << "Mac80211Control Received a wakeup";
    // TODO Implement!
    throw cRuntimeError("wakeup not yet implemented");
    return true;
}
