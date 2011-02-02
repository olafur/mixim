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

#ifndef ARP_H_
#define ARP_H_

#include <ArpInterface.h>
#include <BaseModule.h>

/**
 *
 * @brief An arp module for mapping a mac addres into a network address.
 * Unlike the BaseArp module it does not assume that each node has a single
 * NIC called 'nic'.
 *
 * @author Olafur R. Helgason
 */
class Arp : public ArpInterface, public BaseModule {
public:
    Arp() {};
    virtual ~Arp() {};

    virtual void initialize(int stage);

    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    //virtual void handleMessage( cMessage* ){
    //    error("ARP module cannot receive messages!");
    //};

    /** @brief returns a L2 address to a given L3 address.*/
    virtual int getMacAddr(const int netwAddr);
};

#endif /* ARP_H_ */
