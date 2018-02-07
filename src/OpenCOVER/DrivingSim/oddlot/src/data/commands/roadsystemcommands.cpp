/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   08.07.2010
**
**************************************************************************/

#include "roadsystemcommands.hpp"

#include "src/data/roadsystem/roadsystem.hpp"
#include "src/data/roadsystem/rsystemelement.hpp"
#include "src/data/roadsystem/rsystemelementroad.hpp"
#include "src/data/roadsystem/rsystemelementcontroller.hpp"
#include "src/data/roadsystem/rsystemelementjunction.hpp"
#include "src/data/roadsystem/rsystemelementfiddleyard.hpp"
#include "src/data/roadsystem/junctionconnection.hpp"
#include "src/data/roadsystem/roadlink.hpp"

#include "src/data/roadsystem/track/trackcomponent.hpp"

#include "src/data/visitors/idchangevisitor.hpp"

//#########################//
// AddRoadSystemPrototypeCommand //
//#########################//

AddRoadSystemPrototypeCommand::AddRoadSystemPrototypeCommand(RoadSystem *roadSystem, const RoadSystem *prototypeRoadSystem, const QPointF &deltaPos, double deltaHeadingDegrees, DataCommand *parent)
    : DataCommand(parent)
    , roadSystem_(roadSystem)
{
    // Check for validity //
    //
    if (!roadSystem_ || !prototypeRoadSystem)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Add RoadSystem not successful! NULL argument."));
        return;
    }

    // Transform //
    //
    QTransform transform;
    transform.rotate(deltaHeadingDegrees);

    // RoadSystemElements //
    //
    // Copy the elements so you own them.
    foreach (RSystemElementRoad *road, prototypeRoadSystem->getRoads())
    {
        RSystemElementRoad *clone = road->getClone();
        foreach (TrackComponent *track, clone->getTrackSections())
        {
            track->setLocalTransform(transform.map(track->getLocalPoint(track->getSStart())) + deltaPos, track->getLocalHeading(track->getSStart()) + deltaHeadingDegrees);
        }
        newRoads_.append(clone);
    }
    foreach (RSystemElementController *controller, prototypeRoadSystem->getControllers())
    {
        newControllers_.append(controller->getClone());
    }
    foreach (RSystemElementJunction *junction, prototypeRoadSystem->getJunctions())
    {
        newJunctions_.append(junction->getClone());
    }
    foreach (RSystemElementFiddleyard *fiddleyard, prototypeRoadSystem->getFiddleyards())
    {
        newFiddleyards_.append(fiddleyard->getClone());
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Add RoadSystem"));
}

/*! \brief .
*
*/
AddRoadSystemPrototypeCommand::~AddRoadSystemPrototypeCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        // RoadSystemElements //
        //
        foreach (RSystemElementRoad *road, newRoads_)
        {
            delete road;
        }
        foreach (RSystemElementController *controller, newControllers_)
        {
            delete controller;
        }
        foreach (RSystemElementJunction *junction, newJunctions_)
        {
            delete junction;
        }
        foreach (RSystemElementFiddleyard *fiddleyard, newFiddleyards_)
        {
            delete fiddleyard;
        }
    }
    else
    {
        // nothing to be done (elements are now owned by the road)
    }
}

/*! \brief .
*
*/
void
AddRoadSystemPrototypeCommand::redo()
{
    // Changed Ids //
    //
    QMap<odrID, odrID> roadIds;
    QMap<odrID, odrID> controllerIds;
    QMap<odrID, odrID> junctionIds;
    QMap<odrID, odrID> fiddleyardIds;

    // RoadSystemElements //
    //
    foreach (RSystemElementRoad *road, newRoads_)
    {
        odrID oldId = road->getID();
        roadSystem_->addRoad(road);
        if (oldId != road->getID()) // The Id has been changed.
        {
            roadIds.insert(oldId, road->getID()); // Add to the list of changed Ids.
        }
    }
    foreach (RSystemElementController *controller, newControllers_)
    {
        odrID oldId = controller->getID();
        roadSystem_->addController(controller);
        if (oldId != controller->getID()) // The Id has been changed.
        {
            controllerIds.insert(oldId, controller->getID()); // Add to the list of changed Ids.
        }
    }
    foreach (RSystemElementJunction *junction, newJunctions_)
    {
		odrID oldId = junction->getID();
        roadSystem_->addJunction(junction);
        if (oldId != junction->getID()) // The Id has been changed.
        {
            junctionIds.insert(oldId, junction->getID()); // Add to the list of changed Ids.
        }
    }
    foreach (RSystemElementFiddleyard *fiddleyard, newFiddleyards_)
    {
		odrID oldId = fiddleyard->getID();
        roadSystem_->addFiddleyard(fiddleyard);
        if (oldId != fiddleyard->getID()) // The Id has been changed.
        {
            fiddleyardIds.insert(oldId, fiddleyard->getID()); // Add to the list of changed Ids.
        }
    }

    // If there are Id changes //
    //
    if (!roadIds.empty()
        || !controllerIds.empty()
        || !junctionIds.empty()
        || !fiddleyardIds.empty())
    {
        IdChangeVisitor *visitor = new IdChangeVisitor(roadIds, controllerIds, junctionIds, fiddleyardIds);

        foreach (RSystemElementRoad *road, newRoads_)
        {
            road->accept(visitor);
        }
        foreach (RSystemElementController *controller, newControllers_)
        {
            controller->accept(visitor);
        }
        foreach (RSystemElementJunction *junction, newJunctions_)
        {
            junction->accept(visitor);
        }

        foreach (RSystemElementFiddleyard *fiddleyard, newFiddleyards_)
        {
            fiddleyard->accept(visitor);
        }

        delete visitor;
    }

    setRedone();
}

/*! \brief
*
*/
void
AddRoadSystemPrototypeCommand::undo()
{
    // RoadSystemElements //
    //
    foreach (RSystemElementRoad *road, newRoads_)
    {
        roadSystem_->delRoad(road);
    }
    foreach (RSystemElementController *controller, newControllers_)
    {
        roadSystem_->delController(controller);
    }
    foreach (RSystemElementJunction *junction, newJunctions_)
    {
        roadSystem_->delJunction(junction);
    }
    foreach (RSystemElementFiddleyard *fiddleyard, newFiddleyards_)
    {
        roadSystem_->delFiddleyard(fiddleyard);
    }

    setUndone();
}

//#########################//
// SetRSystemElementIdCommand //
//#########################//

SetRSystemElementIdCommand::SetRSystemElementIdCommand(RoadSystem *roadSystem, RSystemElement *element, const odrID &Id, const QString &name, DataCommand *parent)
    : DataCommand(parent)
    , roadSystem_(roadSystem)
    , element_(element)
    , newId_(Id)
    , newName_(name)
{
    // Check for validity //
    //
    if (!element_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Set element Id: invalid parameters! No element given."));
        return;
    }

    oldId_ = element_->getID();
    oldName_ = element_->getName();

    // Done //
    //
    setValid();
    setText(QObject::tr("Set element Id"));
}

/*! \brief .
*
*/
SetRSystemElementIdCommand::~SetRSystemElementIdCommand()
{
}

/*! \brief .
*
*/
void
SetRSystemElementIdCommand::redo()
{
    roadSystem_->changeUniqueId(element_, newId_);
    element_->setName(newName_);

    setRedone();
}

/*! \brief
*
*/
void
SetRSystemElementIdCommand::undo()
{
    roadSystem_->changeUniqueId(element_, oldId_);
    element_->setName(oldName_);

    setUndone();
}

//#########################//
// AddToJunctionCommand //
//#########################//

AddToJunctionCommand::AddToJunctionCommand(RoadSystem *roadSystem, RSystemElementRoad *road, RSystemElementJunction *junction, DataCommand *parent)
    : roadSystem_(roadSystem)
    , road_(road)
    , junction_(junction)
    , DataCommand(parent)

{
    // Check for validity //
    //
    if (!road_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("AddToJunctionCommand: invalid parameters! No road given."));
        return;
    }
    if (junction_ == NULL)
    {
        junction_ = new RSystemElementJunction("unnamed");
        roadSystem_->addJunction(junction_);
    }

    oldJunctionID_ = road->getJunction();

    // Incoming roads for the connections //
    RoadLink * predecessor = road_->getPredecessor();
    if (predecessor)
    {
        JunctionConnection * connection = new JunctionConnection(QString("%1").arg(junction->getConnections().size()), predecessor->getElementId(), road_->getID(), predecessor->getContactPoint(), 1);
        connections_.append(connection);
    }

    RoadLink * successor = road_->getSuccessor();
    if (successor)
    {
        JunctionConnection * connection = new JunctionConnection(QString("%1").arg(junction->getConnections().size()), successor->getElementId(), road_->getID(), successor->getContactPoint(), 1);
        connections_.append(connection);
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Set Junction"));
}

/*! \brief .
*
*/
AddToJunctionCommand::~AddToJunctionCommand()
{
}

/*! \brief .
*
*/
void
AddToJunctionCommand::redo()
{
    road_->setJunction(junction_->getID());

    for (int i = 0; i < connections_.size(); i ++)
    {
        junction_->addConnection(connections_.at(i));
    }

    /*
		JunctionConnection * connection = new JunctionConnection(id, incomingRoad, connectingRoad, contactPoint, numerator);
		junction->addConnection(connection);
*/

    setRedone();
}

/*! \brief
*
*/
void
AddToJunctionCommand::undo()
{
    road_->setJunction(oldJunctionID_);

    for (int i = 0; i < connections_.size(); i++)
    {
        junction_->delConnection(connections_.at(i));
    }

    setUndone();
}

//#########################//
// RemoveFromJunctionCommand //
//#########################//

RemoveFromJunctionCommand::RemoveFromJunctionCommand(RSystemElementJunction *junction, RSystemElementRoad *road, DataCommand *parent)
    : junction_(junction)
    , road_(road)
    , DataCommand(parent)

{
    // Check for validity //
    //
    if (!road_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("RemoveFromJunctionCommand: invalid parameters! No road given."));
        return;
    }

    oldJunction_ = junction_->getClone();

    // Done //
    //
    setValid();
    setText(QObject::tr("Remove from Junction"));
}

/*! \brief .
*
*/
RemoveFromJunctionCommand::~RemoveFromJunctionCommand()
{
}

/*! \brief .
*
*/
void
RemoveFromJunctionCommand::redo()
{
    auto connectionIterator = oldJunction_->getConnections().constBegin();

    while (connectionIterator != oldJunction_->getConnections().constEnd())
    {
        JunctionConnection *conn = connectionIterator.value();
        if ((conn->getIncomingRoad() == road_->getID()) || (conn->getConnectingRoad() == road_->getID()))
        {
            JunctionConnection *currentConnection = junction_->getConnection(conn->getIncomingRoad(), conn->getConnectingRoad());
            junction_->delConnection(currentConnection);
        }
        connectionIterator++;
    }

    road_->setJunction(odrID::invalidID());

    /*
		JunctionConnection * connection = new JunctionConnection(id, incomingRoad, connectingRoad, contactPoint, numerator);
		junction->addConnection(connection);
*/

    setRedone();
}

/*! \brief
*
*/
void
RemoveFromJunctionCommand::undo()
{
    road_->setJunction(junction_->getID());

    auto newConnections = junction_->getConnections();
    auto connectionIterator = oldJunction_->getConnections().constBegin();
    while (connectionIterator != oldJunction_->getConnections().constEnd())
    {
        if (!newConnections.contains(connectionIterator.key()))
        {
            junction_->addConnection(connectionIterator.value());
        }

        connectionIterator++;
    }

    setUndone();
}
