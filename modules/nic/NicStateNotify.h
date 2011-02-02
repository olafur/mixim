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

#ifndef NICSTATENOTIFY_H_
#define NICSTATENOTIFY_H_

#include "ImNotifiable.h"
#include "IControllable.h"

/**
 * @brief NicStateNotify is published by a controllable nic to announce
 * its status (i.e. on, sleeping or off).
 *
 * @ingroup
 */
class NicStateNotify : public BBItem
{
    BBITEM_METAINFO(BBItem)

public:
    /** @brief ID of the nic that is changing states */
    int nicId;

    /** @brief The new status of the nic */
    IControllable::Status status;
};

#endif
