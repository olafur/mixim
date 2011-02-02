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

#ifndef ICONTROLLABLE_H_
#define ICONTROLLABLE_H_

/**
 * @brief Abstract interface for controllable components.
 */
class IControllable {
public:
    enum Controls {TURN_ON, SLEEP, WAKE_UP, TURN_OFF};
    enum Status {TURNED_ON, SLEEPING, TURNED_OFF};

    virtual ~IControllable() {};
    virtual bool isOn() {return status == TURNED_ON;};
    virtual bool isSleeping() {return status == SLEEPING;};
    virtual bool isOff() {return status == TURNED_OFF;};

protected:
    /** @brief Turns the component on. Returns true on success.*/
    virtual bool turnOn() = 0;
    /** @brief Sends component to sleep. Returns true on success.*/
    virtual bool sleep() = 0;
    /** @brief Wakes up the component. Returns true on success.*/
    virtual bool wakeUp() = 0;
    /** @brief Turns off component on. Returns true on success.*/
    virtual bool turnOff() = 0;

    Status status;
};

#endif /* ICONTROLLABLE_H_ */
