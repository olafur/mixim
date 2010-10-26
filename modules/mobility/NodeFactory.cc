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

#include "NodeFactory.h"
#include "cstringtokenizer.h"
#include <fstream>
#include <sstream>
#include <vector>

// Class TraceEntryParser ------------------------------------------------

bool TraceEntryParser::parse(const string line)
{
    // Returns a vector of the words in the line
    // (default delimiter is " ")
    cStringTokenizer tokenizer(line.c_str());
    vector < string > entries = tokenizer.asVector();
    if (entries.size() < 3)
    {
        ev << "less than three entries in trace line" << endl;
        return false;
    }
    _time = entries[0];
    _cmd = entries[1];
    _nodeID = entries[2];
    if (_cmd == "create")
    {
        if (entries.size() != 6)
        {
            ev << "create line must have 6 entries" << endl;
            return false;
        }
        _eCmd = CREATE;
        _x = entries[3];
        _y = entries[4];
        _z = entries[5];
        return true;
    }
    if (_cmd == "destroy")
    {
        _eCmd = DESTROY;
        return true;
    }
    if (_cmd == "setdest")
    {
        if (entries.size() != 8)
        {
            ev << "setdest line must have 8 entries" << endl;
            return false;
        }
        _eCmd = SETDEST;
        _x = entries[3];
        _y = entries[4];
        _z = entries[5];
        _speed = entries[6];
        _timeAtDest = entries[7];
        return true;
    }
    return true;

}

//#define __NODE_FACTORY_DEBUG__

// The module class needs to be registered with OMNeT++
Define_Module(NodeFactory);

//
// Constructor
//
// Initialize event messages
//
NodeFactory::NodeFactory()
{
    _generateCount = 0;
    _initializedCount = 0;
    _destroyedCount = 0;
    _totalLifetime = 0.0;
    _defaultNodeNamePrefix = "";
}

//
// initialize
//
// Override of virtual base class method.
// Initialize simulation at startup.
//
void NodeFactory::initialize()
{
    _scenarioSizeX = par("scenarioSizeX");
    _scenarioSizeY = par("scenarioSizeY");
    _traceFile = string((const char *) par("traceFile"));
    _defaultNodeNamePrefix = string((const char *) par("defaultNodeNamePrefix"));
    _defaultNodeTypename = string((const char *) par("defaultNodeTypename"));
    _stopAfterLastDestroy = par("stopAfterLastDestroy").boolValue();

    if (_defaultNodeTypename != "")
    {
        _defaultNodeType = cModuleType::get(_defaultNodeTypename.c_str());
    }
    else
    {
        _defaultNodeType = 0;
    }

    // Trace file must be defined. Display error if not.
    if (_traceFile == "")
    {
        throw cRuntimeError("No trace file given for NodeFactory.");
    }
    else
    {
        readSetdestTrace();
    }

    if (ev.isGUI())
    {
        WATCH(_initializedCount);
        WATCH(_generateCount);
        WATCH(_destroyedCount);
    }
}

void NodeFactory::finish()
{
    // Dispose of any remaining dynamically created modules.
    // If GOU is on we do not delete them but let them stay on the display
    if (!ev.isGUI())
    {
        for (unsigned int i = 0; i < _createdItems.size(); i++)
        {
            NodeFactoryItem *item = (NodeFactoryItem *) _createdItems.at(i);
            if (item == NULL)
                continue;
            _totalLifetime = simTime() - item->getCreateTime();
            item->getModule()->callFinish();
            item->getModule()->deleteModule();
            delete item;
            item = 0;
            _destroyedCount++;
        }
    }
}

/**
 * The factory only handles create and destroy events, that is events scheduled in response
 * to scripted events read from a trace file.
 *
 * The simulation is terminated after the last node has been destroyed, when no nodes 
 * remain to be instantiated.
 */
void NodeFactory::handleMessage(cMessage * msg)
{
    if (msg->getKind() == CREATE)
    {
        CreateEvent *te = check_and_cast < CreateEvent * >(msg);
        createNode(te);
        delete msg;
    }
    else if (msg->getKind() == DESTROY)
    {
        DestroyEvent *te = check_and_cast < DestroyEvent * >(msg);
        destroyNode(te);
        delete msg;
        if(_stopAfterLastDestroy && _destroyedCount == _initializedCount)
        {
        	if(!_createdItems.empty())
        	    throw cRuntimeError("Stopping after last destroy but node vector not empty");
            endSimulation();
        }
    }
}

void NodeFactory::createNode(CreateEvent * event)
{
    // Get the module type object from the given class name
    cModuleType *moduleType = 0;
    if (event->getType() != 0 && string(event->getType()) != "")
    {
        moduleType = cModuleType::get(event->getType());
    }

    std::stringstream convert;
    convert << event->getNodeID();
    string nodeID = convert.str();
    string objectName = _defaultNodeNamePrefix + nodeID;
    // create module
    cModule *module = 0;
    ev << "Creating module " << objectName << endl;
    if (moduleType != 0)
    {
        module = moduleType->create(objectName.c_str(), this->getParentModule());
    }
    else
    {
        if (_defaultNodeType == 0)
        {
            throw cRuntimeError("No default node type given and no module type given in create");
        }
        module = _defaultNodeType->create(objectName.c_str(), this->getParentModule());
    }
    module->finalizeParameters();
    module->buildInside();

    // Set initial position in mobility sub-module
    cModule *mobility = module->getSubmodule("mobility");

    bool hasMobility = false;
    if (mobility != 0)
    {
        hasMobility = true;
        if (mobility->hasPar("x"))
            mobility->par("x") = event->getX();
        if (mobility->hasPar("y"))
            mobility->par("y") = event->getY();
        if (mobility->hasPar("z"))
            mobility->par("z") = event->getZ();
    }

    // create activation message
    module->scheduleStart(simTime());
    module->callInitialize();

    // Populate the mobility module of the created nodes with the cached events read from
    // the initial trace file.
    if (_pendingWaypointsLists[event->getNodeID()].size() != 0)
    {
        if (!hasMobility)
        {
            throw cRuntimeError("Waypoints exist for a node with no mobility sub-module.");
        }
        cModuleType *mobilityType = mobility->getModuleType();
        cModuleType *traceMobilityType = 
            cModuleType::get("org.mixim.modules.mobility.TraceMobility");
        if (mobilityType != traceMobilityType)
            throw
                cRuntimeError
                ("Node does not have mobility module of type org.mixim.modules.TraceMobility");

        waypointEventsList waypointList = _pendingWaypointsLists[event->getNodeID()];
        TraceMobility *tm = check_and_cast < TraceMobility * >(mobility);
        tm->initializeTrace(&waypointList);
        _pendingWaypointsLists.erase(event->getNodeID());
    }

    // Store the created module in our dynamic objects list.
    NodeFactoryItem *item = new NodeFactoryItem(module, event->getNodeID(), simTime());
    _createdItems.push_back(item);
    _generateCount++;
}

/**
 * Search through the createdItems list and destroy the node in our dynamic collection
 * whose id matches that of the DestroyEvent.
 *
 * @todo This search could be more elegant.
 */
void NodeFactory::destroyNode(DestroyEvent * event)
{
    cModule *module;
    NodeFactoryItem *item;
    CREATED_ITEMS_VECTOR_TYPE::iterator iter;
    bool deleted = false;
    for (iter = _createdItems.begin(); iter != _createdItems.end(); iter++)
    {
        item = (*iter);
        if (item == NULL)
            continue;
        if (item->getId() == event->getNodeID())
        {
            _totalLifetime += simTime() - item->getCreateTime();
            module = item->getModule();
            module->callFinish();
            module->deleteModule();
            delete item;
            _createdItems.erase(iter);
            deleted = true;
            break;
        }
    }
    if (deleted)
        _destroyedCount++;
    else
        throw cRuntimeError("destroy error: Node %d has not been created (or already destroyed)",
                            event->getNodeID());
}

void NodeFactory::readSetdestTrace()
{
    ifstream tracefilestream;
    tracefilestream.open(_traceFile.c_str());
    if (!tracefilestream.is_open())
    {
        string err = string("Could not open tracefile '") + _traceFile + string("'");
        throw cRuntimeError(err.c_str());
    }
    string line;
    TraceEntryParser tep;
    TraceEvent *curEvent = NULL;
    int linecount = 1;
    simtime_t lastDestroyTime = 0;
    while (!std::getline(tracefilestream, line).eof())
    {
        if (!tep.parse(line))
        {
            ev << "Error parsing tracefile entry in line " << linecount << endl;
            throw cRuntimeError("Error parsing tracefile entry");
        }
        switch (tep.command())
        {
        case CREATE:
            {
                int id = atoi(tep.nodeID().c_str());
                if (_nodeIDs.count(id) != 0)
                {
                    throw cRuntimeError("Create error: Node %s already exists",
                                        tep.nodeID().c_str());
                }
                if (tep.x() < 0 || tep.x() > _scenarioSizeX)
                {
                    throw cRuntimeError("Create: Node x coordinate out of bounds");
                }
                if (tep.y() < 0 || tep.y() > _scenarioSizeY)
                {
                    throw cRuntimeError("Create: Node y coordinate out of bounds");
                }
                curEvent = new CreateEvent();
                curEvent->setKind(CREATE);
                _initializedCount++;
                curEvent->setTime(tep.time());
                curEvent->setNodeID(atoi(tep.nodeID().c_str()));
                ((CreateEvent *) curEvent)->setX(tep.x());
                ((CreateEvent *) curEvent)->setY(tep.y());
                ((CreateEvent *) curEvent)->setZ(tep.z());
                ((CreateEvent *) curEvent)->setType(0);
                _nodeIDs.insert(id);
                scheduleAt(curEvent->getTime(), curEvent);
            }
            break;
        case SETDEST:
            {
                if (tep.x() < 0 || tep.x() > _scenarioSizeX)
                {
                    throw cRuntimeError("setdest: Node x coordinate out of bounds");
                }
                if (tep.y() < 0 || tep.y() > _scenarioSizeY)
                {
                    throw cRuntimeError("setdest: Node y coordinate out of bounds");
                }
                SetDestEvent curWaypointEvent;
                //WAYPOINT_EVENT curWaypointEvent;
                curWaypointEvent.setTime(tep.time());
                curWaypointEvent.setNodeID(atoi(tep.nodeID().c_str()));
                curWaypointEvent.setSpeed(tep.speed());
                curWaypointEvent.setX(tep.x());
                curWaypointEvent.setY(tep.y());
                curWaypointEvent.setZ(tep.z());
                curWaypointEvent.setTimeAtDest(tep.timeAtDest());
                if (_pendingWaypointsLists.count(curWaypointEvent.getNodeID()) == 0){
                    waypointEventsList el;
                    _pendingWaypointsLists[curWaypointEvent.getNodeID()] = el;
                }
                _pendingWaypointsLists[curWaypointEvent.getNodeID()].push_back(curWaypointEvent);
            }
            break;
        case DESTROY:
            {
                int id = atoi(tep.nodeID().c_str());
                if (_nodeIDs.count(id) != 1)
                {
                    error("Destroy error: Node %s not existing or already destroyed",
                          tep.nodeID().c_str());
                }
                _nodeIDs.erase(id);
                curEvent = new DestroyEvent();
                curEvent->setKind(DESTROY);
                curEvent->setTime(tep.time());
                curEvent->setNodeID(atoi(tep.nodeID().c_str()));
                scheduleAt(curEvent->getTime(), curEvent);
            }
            break;
        default:
            throw cRuntimeError("Unknown command in tracefile");
        }
        linecount++;
    }
    tracefilestream.close();
}
