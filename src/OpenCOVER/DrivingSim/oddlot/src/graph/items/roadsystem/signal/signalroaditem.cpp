/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   12.03.2010
**
**************************************************************************/

#include "signalroaditem.hpp"

#include "src/util/odd.hpp"
#include "src/util/colorpalette.hpp"

#include "src/mainwindow.hpp"

// Data //
//
#include "src/data/roadsystem/rsystemelementroad.hpp"
#include "src/data/roadsystem/sections/signalobject.hpp"
#include "src/data/roadsystem/sections/tunnelobject.hpp"
#include "src/data/roadsystem/sections/lanesection.hpp"
#include "src/data/roadsystem/sections/lane.hpp"

#include "src/data/commands/dataelementcommands.hpp"
#include "src/data/commands/signalcommands.hpp"

// Widget //
//
#include "src/gui/projectwidget.hpp"

// Graph //
//

#include "src/graph/items/roadsystem/roadtextitem.hpp"
#include "src/graph/items/roadsystem/roadsystemitem.hpp"
#include "src/graph/items/roadsystem/signal/signalitem.hpp"
#include "src/graph/items/roadsystem/signal/objectitem.hpp"
#include "src/graph/items/roadsystem/signal/bridgeitem.hpp"
#include "src/graph/items/roadsystem/signal/signalhandle.hpp"
#include "src/graph/editors/signaleditor.hpp"

// SignalManager //
//
#include "src/data/signalmanager.hpp"

// Qt //
//
#include <QBrush>
#include <QPen>
#include <QCursor>

SignalRoadItem::SignalRoadItem(RoadSystemItem *roadSystemItem, RSystemElementRoad *road)
    : RoadItem(roadSystemItem, road)
    , roadSystemItem_(roadSystemItem)
    , road_(road)
{
    init();
}

SignalRoadItem::~SignalRoadItem()
{
}

void
SignalRoadItem::init()
{
    // Hover Events //
    //
    setAcceptHoverEvents(true);
	ProjectWidget *projectWidget = getProjectGraph()->getProjectWidget();
    signalEditor_ = dynamic_cast<SignalEditor *>(projectWidget->getProjectEditor());
	signalManager_ = projectWidget->getMainWindow()->getSignalManager();


    foreach (Signal *signal, road_->getSignals())
    {
        new SignalItem(roadSystemItem_, signal, road_->getGlobalPoint(signal->getSStart(), signal->getT()));
    }

    foreach (Object *object, road_->getObjects())
    {
        new ObjectItem(roadSystemItem_, object, road_->getGlobalPoint(object->getSStart(), object->getT()));
    }

    foreach (Bridge *bridge, road_->getBridges())
    {
        new BridgeItem(roadSystemItem_, bridge, road_->getGlobalPoint(bridge->getSStart(), 0.0));
    }

    updateColor();
    createPath();
}
/*! \brief Sets the color according to the number of links.
*/
void
SignalRoadItem::updateColor()
{

    setBrush(QBrush(ODD::instance()->colors()->brightBlue()));
    setPen(QPen(ODD::instance()->colors()->darkBlue()));
}

void
SignalRoadItem::createPath()
{
    RSystemElementRoad *road = getRoad();

    // Initialization //
    //
    double sStart = 0.0;
    double sEnd = road->getLength();
    if (sEnd < sStart)
        sEnd = sStart;

    //	double pointsPerMeter = 1.0; // BAD: hard coded!
    double pointsPerMeter = getProjectGraph()->getProjectWidget()->getLODSettings()->TopViewEditorPointsPerMeter;
    int pointCount = int(ceil((sEnd - sStart) * pointsPerMeter)); // TODO curvature...
    if (pointCount < 2)
    {
        pointCount = 2;
    }
    QVector<QPointF> points(2 * pointCount + 1);
    double segmentLength = (sEnd - sStart) / (pointCount - 1);

    // Right side //
    //
    for (int i = 0; i < pointCount; ++i)
    {
        double s = sStart + i * segmentLength; // [sStart, sEnd]
        points[i] = road->getGlobalPoint(s, road->getMinWidth(s));
    }

    // Left side //
    //
    for (int i = 0; i < pointCount; ++i)
    {
        double s = sEnd - i * segmentLength; // [sEnd, sStart]
        if (s < 0.0)
            s = 0.0; // can happen due to numerical inaccuracy (around -1.0e-15)
        points[i + pointCount] = road->getGlobalPoint(s, road->getMaxWidth(s));
    }

    // End point //
    //
    points[2 * pointCount] = road->getGlobalPoint(sStart, road->getMinWidth(sStart));

    // Psycho-Path //
    //
    QPainterPath path;
    path.addPolygon(QPolygonF(points));

    setPath(path);
}

//################//
// EVENTS         //
//################//

void
SignalRoadItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    ODD::ToolId tool = signalEditor_->getCurrentTool();
    /*	if(tool == ODD::TRT_SELECT)
	{
		// Parent: selection //
		//
		return SectionItem::mousePressEvent(event);
	}
	else
	{
		return; // prevent selection by doing nothing
	}*/
}

void
SignalRoadItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    ODD::ToolId tool = signalEditor_->getCurrentTool();
    /*	if(tool == ODD::TRT_SELECT)
	{
		// Parent: selection //
		//
		SectionItem::mouseReleaseEvent(event);
	}*/

    pos_ = event->scenePos();
    if ((tool == ODD::TSG_SIGNAL) || (tool == ODD::TSG_OBJECT))
    {
        // Add new Signal //
        //
        double s = road_->getSFromGlobalPoint(pos_, 0.0, road_->getLength());

        // calculate t at s
        //
		QVector2D vec = QVector2D(road_->getGlobalPoint(s) - pos_);
		double t = vec.length();
		if (QVector2D::dotProduct(road_->getGlobalNormal(s), vec) < 0)
		{
			t = -t;
		}

        if (tool == ODD::TSG_SIGNAL)
        {
			signalEditor_->addSignalToRoad(road_, s, t);
        }
        else if (tool == ODD::TSG_OBJECT)
        {
			signalEditor_->addObjectToRoad(road_, s, t);
        }
    }

	else if (tool == ODD::TSG_BRIDGE)
	{
		double s = road_->getSFromGlobalPoint(pos_, 0.0, road_->getLength());

		// Add new bridge //
		//
		Bridge *newBridge = new Bridge("bridge", "", "", Bridge::BT_CONCRETE, s, 100.0);
		AddBridgeCommand *command = new AddBridgeCommand(newBridge, road_, NULL);

		getProjectGraph()->executeCommand(command);
	}
	else if (tool == ODD::TSG_TUNNEL)
	{
		double s = road_->getSFromGlobalPoint(pos_, 0.0, road_->getLength());

		// Add new bridge //
		//
		Tunnel *newTunnel = new Tunnel("tunnel", "", "", Tunnel::TT_STANDARD, s, 100.0, 0.0, 0.0);
		AddBridgeCommand *command = new AddBridgeCommand(newTunnel, road_, NULL);

		getProjectGraph()->executeCommand(command);
	}
}

void
SignalRoadItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    ODD::ToolId tool = signalEditor_->getCurrentTool();
    if (tool == ODD::TSG_SELECT)
    {
        // Parent: selection //
        //
        //	SectionItem::mouseMoveEvent(event); // pass to baseclass
    }
    else if (tool == ODD::TSG_SIGNAL)
    {
        // does nothing //
    }
    else if (tool == ODD::TSG_OBJECT)
    {
        // does nothing //
    }
}

void
SignalRoadItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    /*	ODD::ToolId tool = signalEditor_->getCurrentTool();
	if(tool == ODD::TSG_SELECT)
	{
		setCursor(Qt::PointingHandCursor);
	}
	else if((tool == ODD::TSG_SIGNAL) || (tool == ODD::TSG_OBJECT))
	{
		setCursor(Qt::CrossCursor);
		signalEditor_->getInsertSignalHandle()->show();
	}
	else if(tool == ODD::TSG_DEL)
	{
		setCursor(Qt::CrossCursor);
	}

	// Text //
	//
	getTextItem()->setVisible(true);
	getTextItem()->setPos(event->scenePos());
*/
	// Parent //
	//
	RoadItem::hoverEnterEvent(event); // pass to baseclass

	
}

void
SignalRoadItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    /*	ODD::ToolId tool = signalEditor_->getCurrentTool();
	if(tool == ODD::TSG_SELECT)
	{
		// does nothing //
	}
	else if((tool == ODD::TSG_SIGNAL) || (tool == ODD::TSG_OBJECT))
	{
		signalEditor_->getInsertSignalHandle()->hide();
	}
	else if(tool == ODD::TSG_DEL)
	{
		// does nothing //
	}

	setCursor(Qt::ArrowCursor);

	// Text //
	//
	getTextItem()->setVisible(false);
*/
	// Parent //
	//
	RoadItem::hoverLeaveEvent(event); // pass to baseclass

	
}

void
SignalRoadItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    /*	ODD::ToolId tool = signalEditor_->getCurrentTool();
	if(tool == ODD::TSG_SELECT)
	{
		// does nothing //
	}
	else if((tool == ODD::TSG_SIGNAL) || (tool == ODD::TSG_OBJECT))
	{
		signalEditor_->getInsertSignalHandle()->updatePos(this, event->scenePos(), 0.0, road_->getLength());
	}
	else if(tool == ODD::TSG_DEL)
	{
		// does nothing //
	}
*/
	// Parent //
	//
	RoadItem::hoverMoveEvent(event);

	
}

//##################//
// Observer Pattern //
//##################//

void
SignalRoadItem::updateObserver()
{
    // Parent //
    //
    GraphElement::updateObserver();
    if (isInGarbage())
    {
        return; // will be deleted anyway
    }

    // Get change flags //
    //
    int changes = road_->getRoadChanges();

    if (changes & RSystemElementRoad::CRD_SignalChange)
    {
        // A signal has been added.
        //
        QMultiMap<double, Signal *>::const_iterator iter = road_->getSignals().constBegin();
        while (iter != road_->getSignals().constEnd())
        {
            Signal *element = iter.value();
            if ((element->getDataElementChanges() & DataElement::CDE_DataElementCreated)
                || (element->getDataElementChanges() & DataElement::CDE_DataElementAdded))
            {
                new SignalItem(roadSystemItem_, element, pos_);
            }

            iter++;
        }
	}

    if (changes & RSystemElementRoad::CRD_ObjectChange)
    {
        // A object has been added.
        //
        QMultiMap<double, Object *>::const_iterator iter = road_->getObjects().constBegin();
        while (iter != road_->getObjects().constEnd())
        {
            Object *element = iter.value();
            if ((element->getDataElementChanges() & DataElement::CDE_DataElementCreated)
                || (element->getDataElementChanges() & DataElement::CDE_DataElementAdded))
            {
                new ObjectItem(roadSystemItem_, element, pos_);
            }

            iter++;
        }
    }

    if (changes & RSystemElementRoad::CRD_BridgeChange)
    {
        // A bridge has been added.
        //
        QMultiMap<double, Bridge *>::const_iterator iter = road_->getBridges().constBegin();
        while (iter != road_->getBridges().constEnd())
        {
            Bridge *element = iter.value();
            if ((element->getDataElementChanges() & DataElement::CDE_DataElementCreated)
                || (element->getDataElementChanges() & DataElement::CDE_DataElementAdded))
            {
                new BridgeItem(roadSystemItem_, element, pos_);
            }

            iter++;
        }
    }
}
