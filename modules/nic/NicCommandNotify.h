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

#ifndef NICCOMMANDNOTIFY_H_
#define NICCOMMANDNOTIFY_H_

#include "ImNotifiable.h"
#include "IControllable.h"

/**
 * @brief NicCommandNotify is published by an application to send a
 * control message to a nic (i.e. TURN_ON, TURN_OFF or SLEEP)
 *
 * @ingroup
 */
class NicCommandNotify : public BBItem
{
    BBITEM_METAINFO(BBItem)

public:
    /** @brief ID of the nic that the command is directed to */
    int nicId;

    /** @brief The control message to send to the nic */
    IControllable::Controls command;
};

#endif
