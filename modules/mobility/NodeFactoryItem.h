// 
// Copyright (C) 2008 Olafur Ragnar Helgason, Laboratory for Communication 
//                    Networks, KTH, Stockholm
//           (C)      Kristjan Valur Jonsson, Reykjavik University, Reykjavik
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

#ifndef __NODE_FACTORY_ITEM_INCLUDED__
#define __NODE_FACTORY_ITEM_INCLUDED__

#include <omnetpp.h>

/**
 * @brief Data structure to hold a single dynamically created node in the simulation
 *        along with some basic data on the module.
 */
class NodeFactoryItem
{
	private:
		int       m_id;
		cModule  *m_module;
		simtime_t m_tCreateTime;
    
	public:
		NodeFactoryItem();
		NodeFactoryItem( cModule *module, int id, simtime_t createTime );

	public:
    int       getId() { return m_id; }
    cModule * getModule() { return m_module; }
    simtime_t getCreateTime() { return m_tCreateTime; }
};

#endif /* __NODE_FACTORY_ITEM_INCLUDED__ */
