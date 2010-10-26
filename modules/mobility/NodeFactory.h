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

#ifndef __NODE_FACTORY_INCLUDED__
#define __NODE_FACTORY_INCLUDED__

#include <omnetpp.h>
#include <string>
#include "NodeFactoryItem.h"
#include "TraceEvents_m.h"
#include "TraceMobility.h"

using namespace std;

enum Commands
{ CREATE, SETDEST, DESTROY, UNKNOWN };
typedef vector < NodeFactoryItem * >CREATED_ITEMS_VECTOR_TYPE;

class TraceEntryParser
{
  public:
    TraceEntryParser():_eCmd(UNKNOWN)
    {
    }

    bool parse(string line);

    int command() const
    {
        return _eCmd;
    }
    simtime_t time() const
    {
        return SimTime::parse(_time.c_str());
    }
    string nodeID() const
    {
        return _nodeID;
    }

    double x() const
    {
        return atof(_x.c_str());
    }
    double y() const
    {
        return atof(_y.c_str());
    }
    double z() const
    {
        return atof(_z.c_str());
    }
    double speed() const
    {
        return atof(_speed.c_str());
    }
    simtime_t timeAtDest() const
    {
        return SimTime::parse(_timeAtDest.c_str());
    };

  private:
    string _time;
    string _cmd;
    string _nodeID;
    string _x, _y, _z;
    string _speed;
    string _timeAtDest;

    Commands _eCmd;
};

/**
 *
 * @brief Node factory object. Creates nodes dynamically from a 
 * tracefile. A mobility trace defines create, destroy and waypoint 
 * events for nodes. 
 *
 * @author Kristjan V. Jonsson
 * @author Olafur R. Helgason
 */
class NodeFactory:public cSimpleModule
{
  private:
    int _scenarioSizeX;                                    /**< @brief The width of the scenario */
    int _scenarioSizeY;                                    /**< @brief The height of the scenario */
    string _traceFile;                 /**< @brief Name of the tracefile */
    string _defaultNodeNamePrefix;         /** < @brief Default name prefix of generated nodes*/
    string _defaultNodeTypename;         /** < @brief Name of default node type*/
    cModuleType *_defaultNodeType;    /** < @brief ptr to default node creator*/
    bool _stopAfterLastDestroy;

    set < int >_nodeIDs;

    /** @brief The number of initialized modules, i.e. create commands read from file */
    unsigned long _initializedCount;
    /** @brief The number of generated modules */
    unsigned long _generateCount;
    /** @brief The number of destroyed modules. The simulation ends when the number of 
        destroyed modules equals that of generated ones */
    unsigned long _destroyedCount;
    /** @brief The total lifetime of generated nodes. Used to calculate average lifetime */
    simtime_t _totalLifetime;

    /** @brief The generated modules */
    CREATED_ITEMS_VECTOR_TYPE _createdItems;

    /** @brief A collection of lists of waypoints read from the trace file. The lists
               are assigned to TraceMobility modules of created nodes and then
               cleared. */
      map < int, waypointEventsList > _pendingWaypointsLists;

  public:
    /** @brief Constructor */
      NodeFactory();

  protected:
        /** @brief Overrides of virtual base class functions. */
      virtual void initialize();
    /** @brief Overrides of virtual base class functions. */
    virtual void finish();
    /** @brief Overrides of virtual base class functions. Handles create and destroy messages. */
    virtual void handleMessage(cMessage * msg);

    /** @brief Create a node. Triggered by a CreateEvent message */
    void createNode(CreateEvent * event);
    /** @brief Destroy a node. Triggered by a DestroyEvent message */
    void destroyNode(DestroyEvent * event);

    void readSetdestTrace();
    /** @brief Reads a setdest mobility trace. */
};

#endif /* __NODE_FACTORY_INCLUDED__ */
