/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   15.03.2010
**
**************************************************************************/

#include "osceditor.hpp"

#include <math.h>

// Project //
//
#include "src/gui/projectwidget.hpp"

//MainWindow //
//
#include "src/mainwindow.hpp"

// Data //
//
#include "src/data/projectdata.hpp"
#include "src/data/signalmanager.hpp"
#include "src/data/oscsystem/oscelement.hpp"
#include "src/data/oscsystem/oscbase.hpp"
#include "src/data/roadsystem/roadsystem.hpp"
#include "src/data/changemanager.hpp"

// Commands //
//
#include "src/data/commands/osccommands.hpp"
#include "src/data/commands/signalcommands.hpp"
#include "src/data/commands/roadsectioncommands.hpp"
#include "src/data/commands/dataelementcommands.hpp"

// Graph //
//
#include "src/graph/topviewgraph.hpp"
#include "src/graph/graphscene.hpp"
#include "src/graph/graphview.hpp"

#include "src/graph/items/roadsystem/scenario/oscroadsystemitem.hpp"
#include "src/graph/items/oscsystem/oscitem.hpp"
#include "src/graph/items/oscsystem/oscshapeitem.hpp"
#include "src/graph/items/oscsystem/oscbaseitem.hpp"

// Tools //
//
#include "src/gui/tools/osceditortool.hpp"
#include "src/gui/mouseaction.hpp"

// Tree //
//
#include "src/tree/catalogtreewidget.hpp"

// Visitor //
//
//#include "src/graph/visitors/roadmarkvisitor.hpp"

// OpenScenario //
//
#include "oscObjectBase.h"
#include "schema/oscObject.h"
#include "schema/oscCatalogs.h"
#include "oscCatalog.h"
#include "schema/oscPosition.h"
#include "oscArrayMember.h"
#include "schema/oscCatalogReference.h"
#include "oscArrayMember.h"
#include "schema/oscTrajectory.h"
#include "schema/oscActions.h"
#include "schema/oscPrivateAction.h"
#include "schema/oscPrivate.h"

// Boost //
//
#include <boost/filesystem.hpp>


// Qt //
//
#include <QGraphicsItem>
#include <QUndoStack>
#include <QStatusBar>

using namespace OpenScenario;

//################//
// CONSTRUCTORS   //
//################//

OpenScenarioEditor::OpenScenarioEditor(ProjectWidget *projectWidget, ProjectData *projectData, TopviewGraph *topviewGraph)
    : ProjectEditor(projectWidget, projectData, topviewGraph)
	, oscBaseItem_(NULL)
	, oscBase_(NULL)
	, oscCatalog_(NULL)
    , trajectoryElement_(NULL)
{
	mainWindow_ = projectWidget->getMainWindow();
	oscBase_ = projectData->getOSCBase();
	openScenarioBase_ = oscBase_->getOpenScenarioBase();
    oscRoadSystemItem_ = NULL;
}

OpenScenarioEditor::~OpenScenarioEditor()
{
    kill();
}

//################//
// FUNCTIONS      //
//################//

/**
*
*/
void
OpenScenarioEditor::init()
{
	OpenScenario::oscEntities *entitiesObject = openScenarioBase_->Entities.getOrCreateObject();
	OpenScenario::oscArrayMember *oscObjectArray = dynamic_cast<OpenScenario::oscArrayMember *>(entitiesObject->getMember("Object"));
	
	for (int i = 0; i < oscObjectArray->size(); i++)
	{
		OpenScenario::oscObject *object = dynamic_cast<OpenScenario::oscObject *>(oscObjectArray->at(i));
		OpenScenario::oscCatalogReference *catalogReference = object->CatalogReference.getOrCreateObject();

		if (!oscBase_->getOSCElement(object))
		{
			OSCElement *element = new OSCElement(QString::fromStdString(catalogReference->name.getValue()), object);
			oscBase_->addOSCElement(element);
		}
	}

    OpenScenario::oscStoryboard *story = dynamic_cast<OpenScenario::oscStoryboard*>(openScenarioBase_->Storyboard.getObject());
    if (story)
    {
        QList<OpenScenario::oscObjectBase *>trajectoryObjects = getElements(story, "oscTrajectory"); 
        if (!trajectoryObjects.isEmpty())
        {
            OpenScenario::oscCatalogs *catalogs = openScenarioBase_->Catalogs.getOrCreateObject();
            OpenScenario::oscCatalog *trajectoryCatalog = catalogs->TrajectoryCatalog.getOrCreateObject();
            if (trajectoryCatalog->getObjectsMap().size() == 0)
            {
                //get all catalog object filenames
                std::vector<bf::path> filenames = trajectoryCatalog->getXoscFilesFromDirectory(trajectoryCatalog->Directory->path.getValue());

                //parse all files
                //store object name and filename in map
                trajectoryCatalog->fastReadCatalogObjects(filenames);
            }

            foreach (OpenScenario::oscObjectBase *objectBase, trajectoryObjects)
            {
                OpenScenario::oscTrajectory *objectTrajectory = dynamic_cast<OpenScenario::oscTrajectory *>(objectBase);
                std::string name = objectTrajectory->name.getValue();
                OpenScenario::oscTrajectory *catalogObject = dynamic_cast<OpenScenario::oscTrajectory *>(trajectoryCatalog->getCatalogObject(name));
                if (!catalogObject)
                {
                    trajectoryCatalog->fullReadCatalogObjectWithName(name);
                    catalogObject = dynamic_cast<OpenScenario::oscTrajectory *>(trajectoryCatalog->getCatalogObject(name));
                }

				OSCElement *element = new OSCElement(QString::fromStdString(name), catalogObject);
                oscBase_->addOSCElement(element);
            }
        }
    }

	if (!oscRoadSystemItem_)
	{
		// Root item //
        //
        oscRoadSystemItem_ = new OSCRoadSystemItem(getTopviewGraph(), getProjectData()->getRoadSystem());
        getTopviewGraph()->getScene()->addItem(oscRoadSystemItem_);
	}

	// Reset change //
    //

    getProjectData()->getChangeManager()->notifyObservers();


	if (!oscBaseItem_ && openScenarioBase_->getSource())   // OpenScenario Editor graphischee Elemente
    {
		// Root OSC item //
		//
		oscBaseItem_ = new OSCBaseItem(getTopviewGraph(), oscBase_);
		getTopviewGraph()->getScene()->addItem(oscBaseItem_);
	}


        // Signal Handle //
        //
 /*       insertOSCHandle_ = new OSCHandle(signalRoadSystemItem_);
        insertOSCHandle_->hide();*/


    lastTool_ = getCurrentTool();

}

/*!
*/
void
OpenScenarioEditor::kill()
{
	if (oscRoadSystemItem_)
	{
		delete oscRoadSystemItem_;
		oscRoadSystemItem_ = NULL;
	}

	if (oscBaseItem_)
	{
		delete oscBaseItem_;
		oscBaseItem_ = NULL;
	}

}

/*OSCHandle *
OpenScenarioEditor::getOSCHandle() const
{
    if (!insertOSCHandle_)
    {
        qDebug("ERROR 1003281422! OpenScenarioEditor not yet initialized.");
    }
    return insertOSCHandle_;
}*/

// Move Object //
//
QList<OpenScenario::oscObjectBase *>
OpenScenarioEditor::getElements(oscObjectBase *root, const std::string &type)
{
    QList<OpenScenario::oscObjectBase *>objectList;
    OpenScenario::oscObjectBase::MemberMap members = root->getMembers();
    for(OpenScenario::oscObjectBase::MemberMap::iterator it = members.begin();it != members.end();it++)
    {
        oscMember *member = it->second;
        if (member)
        {
            oscObjectBase *objectBase = member->getObject();
            if (objectBase)
            {
                OpenScenario::oscArrayMember *arrayMember = dynamic_cast<OpenScenario::oscArrayMember *>(objectBase->getOwnMember());
                if (arrayMember)
                {
                    for (int i = 0; i < arrayMember->size(); i++)
                    {
                        objectList.append(getElements(arrayMember->at(i), type));
                    }
                }
                else
                {
                    if (member->getTypeName() == type)
                    {
                        objectList.append(objectBase);
                    }
                    objectList.append(getElements(objectBase, type));
                }
            }
        }
    }

    return objectList;
}

bool 
OpenScenarioEditor::translateObject(OpenScenario::oscPrivateAction * privateAction, const QString &newRoadId, double s, double t)
{
	OSCElement *oscElement = oscBase_->getOrCreateOSCElement(privateAction);

	OpenScenario::oscPosition *oscPosition = privateAction->Position.getOrCreateObject();
	OpenScenario::oscRoad *oscPosRoad = oscPosition->Road.getOrCreateObject();
	oscStringValue *roadId = dynamic_cast<oscStringValue *>(oscPosRoad->roadId.getOrCreateValue());

	getProjectData()->getUndoStack()->beginMacro(QObject::tr("Move Object"));

	bool parentChanged = false;
	if (QString::fromStdString(roadId->getValue()) != newRoadId)
	{
		SetOSCValuePropertiesCommand<std::string> *command = new SetOSCValuePropertiesCommand<std::string>(oscElement, oscPosRoad, "roadId", newRoadId.toStdString());
		getProjectGraph()->executeCommand(command);

		parentChanged = true;
		//		object->setElementSelected(false);
	}


	SetOSCValuePropertiesCommand<double> *command = new SetOSCValuePropertiesCommand<double>(oscElement, oscPosRoad, "s", s);
	getProjectGraph()->executeCommand(command);
	command = new SetOSCValuePropertiesCommand<double>(oscElement, oscPosRoad, "t", t);
	getProjectGraph()->executeCommand(command);

    getProjectData()->getUndoStack()->endMacro();

    return parentChanged;
}

/*Object *
OpenScenarioEditor::addObjectToRoad(RSystemElementRoad *road, double s, double t)
{
	ObjectContainer *lastObject = signalManager_->getSelectedObjectContainer();
	Object *newObject = NULL;

	if (lastObject)
	{
		if (t < 0)
		{
			t -= lastObject->getObjectDistance();
		}
		else
		{
			t += lastObject->getObjectDistance();
		}
		newObject = new Object("object", "", lastObject->getObjectType(), s, t, 0.0, 0.0, Object::NEGATIVE_TRACK_DIRECTION, lastObject->getObjectLength(), 
			lastObject->getObjectWidth(), lastObject->getObjectRadius(), lastObject->getObjectHeight(), lastObject->getObjectHeading(),
					0.0, 0.0, false, 0.0, 0.0, lastObject->getObjectRepeatDistance(), lastObject->getObjectFile());
		AddObjectCommand *command = new AddObjectCommand(newObject, road, NULL);
		getProjectGraph()->executeCommand(command);
	}
	else
	{
		Object *newObject = new Object("object", "", "", s, t, 0.0, 0.0, Object::NEGATIVE_TRACK_DIRECTION, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, false, s, 0.0, 0.0, "");
		AddObjectCommand *command = new AddObjectCommand(newObject, road, NULL);
		getProjectGraph()->executeCommand(command);
	}

	return newObject;
}
*/

void 
OpenScenarioEditor::catalogChanged(OpenScenario::oscCatalog * member)
{
	oscCatalog_ = member;
}

OpenScenario::oscCatalog *
OpenScenarioEditor::getCatalog(std::string name)
{

	OpenScenario::oscCatalogs *catalogs = openScenarioBase_->Catalogs.getOrCreateObject();
	OpenScenario::oscCatalog *catalog = dynamic_cast<OpenScenario::oscCatalog *>(catalogs->getMember(name)->getOrCreateObject());
	QString catalogDir = OSCSettings::instance()->getCatalogDir();
	std::string catalogsDir = catalogDir.toStdString(); 
	if (!bf::exists(bf::path(catalogsDir)))
	{
		bf::create_directory(catalogsDir);
	}

	OpenScenario::oscDirectory *dir = dynamic_cast<oscDirectory *>(catalog->getObjectByName("Directory"));
	if (dir)
	{
		std::string dirName = catalog->Directory->path.getValue();
		if (dirName == "")
		{
			dirName = catalog->Directory->path = catalogsDir + name;
			if (!bf::exists(bf::path(dirName)))
			{
				bf::create_directory(dirName);
			}
		}
	}


	name = name.erase(name.find("Catalog"));
	catalog->setCatalogName(name);

	OSCElement *oscElement = oscBase_->getOrCreateOSCElement(catalog);

	return catalog;
}

void
    OpenScenarioEditor::addGraphToObserver(const QVector<QPointF> &controlPoints)
{
    QList<DataElement *> elements = getProjectData()->getSelectedElements();
    for (int i = 0; i < elements.size(); i++)
    {
		OSCElement *element = dynamic_cast<OSCElement *>(elements.at(i));
		if (element)
		{
			OpenScenario::oscTrajectory *trajectory = dynamic_cast<OpenScenario::oscTrajectory *>(element->getObject());

			if (trajectory)
			{
				createWaypoints(trajectory, controlPoints);
			}
		}
	}
}

void 
OpenScenarioEditor::createWaypoints(OpenScenario::oscTrajectory *trajectory, const QVector<QPointF> &controlPoints)
{
    OpenScenario::oscArrayMember *waypointArray = dynamic_cast<OpenScenario::oscArrayMember *>(trajectory->getOwnMember());
    waypointArray->clear();

    getProjectData()->getUndoStack()->beginMacro(QObject::tr("Create waypoints"));
    for (int i = 0; i < controlPoints.count(); i += 3)
    {
        OpenScenario::oscVertex *vertex = trajectory->Vertex.createObject();
        OpenScenario::oscPosition *position = vertex->Position.createObject();
        OpenScenario::oscWorld *posWorld = position->World.createObject();
        posWorld->x.setValue(controlPoints.at(i).x());
        posWorld->y.setValue(controlPoints.at(i).y());

        OpenScenario::oscShape *shape = vertex->Shape.createObject();
        OpenScenario::oscSpline *spline = shape->Spline.createObject();
        int index = i - 1;
        if (index > 0)
        {
            OpenScenario::oscControlPoint1 *controlPoint = spline->ControlPoint1.createObject();
            char buf[100];

            sprintf(buf, "%d %d", qRound(controlPoints.at(index).x()), qRound(controlPoints.at(index).y()));
            controlPoint->status.setValue(buf);
        }

        index = i + 1;
        if (index < controlPoints.count())
        {
            OpenScenario::oscControlPoint2 *controlPoint = spline->ControlPoint2.createObject();
            char buf[100];
            sprintf(buf, "%d %d", qRound(controlPoints.at(index).x()), qRound(controlPoints.at(index).y()));
            controlPoint->status.setValue(buf);
        }

		AddOSCArrayMemberCommand *command = new AddOSCArrayMemberCommand(waypointArray, trajectory, vertex, "vertex", oscBase_, NULL);
        getProjectGraph()->executeCommand(command);
    }
    OSCElement *element = oscBase_->getOrCreateOSCElement(trajectory);
    element->addOSCElementChanges(OSCElement::COE_ParameterChange);

    UnhideDataElementCommand *command = new UnhideDataElementCommand(element);
    getProjectGraph()->executeCommand(command);

    SelectDataElementCommand *selectCommand = new SelectDataElementCommand(element);
    getProjectGraph()->executeCommand(selectCommand);

    getProjectData()->getUndoStack()->endMacro();

    getTopviewGraph()->getView()->clearSplineControlPoints();
}


OpenScenario::oscPrivateAction *
OpenScenarioEditor::getOrCreatePrivateAction(const std::string &selectedObjectName)
{
	OpenScenario::oscStoryboard *story = openScenarioBase_->Storyboard.getOrCreateObject();
	OpenScenario::oscInit *init = story->Init.getOrCreateObject();
	OpenScenario::oscActions *actions = init->Actions.getOrCreateObject();
	OpenScenario::oscArrayMember *privateArray = dynamic_cast<OpenScenario::oscArrayMember *>(actions->getOwnMember());
	OpenScenario::oscPrivate *privateObject = NULL;

	for(oscArrayMember::iterator it =privateArray->begin();it != privateArray->end();it++)
	{
		OpenScenario::oscPrivate *priv = dynamic_cast<OpenScenario::oscPrivate *>(*it);
		if (priv->object.getValue() == selectedObjectName)
		{
			privateObject = priv;
		}
	}

	if (!privateObject)
	{
		OSCElement *oscElement = new OSCElement("Private");
		if (oscElement)
		{
			AddOSCArrayMemberCommand *command = new AddOSCArrayMemberCommand(privateArray, actions, NULL, "Private", oscBase_, oscElement);
			getProjectGraph()->executeCommand(command);
		}
		privateObject = dynamic_cast<OpenScenario::oscPrivate *>(oscElement->getObject());
		privateObject->object.setValue(selectedObjectName);
	}

	OpenScenario::oscPrivateAction *privateActions = privateObject->Action.getOrCreateObject();
	OpenScenario::oscArrayMember *privateActionsArray = dynamic_cast<OpenScenario::oscArrayMember *>(privateActions->getOwnMember());
	OpenScenario::oscPrivateAction *privateAction = NULL;

	for(oscArrayMember::iterator it =privateActionsArray->begin();it != privateActionsArray->end();it++)
	{
		OpenScenario::oscPrivateAction *action = dynamic_cast<OpenScenario::oscPrivateAction *>(*it);

		if (action->getChosenMember()->getName() == "Position")
		{
			privateAction = action;
			break;
		}
	}

	if (!privateAction)
	{
		OSCElement *oscElement = new OSCElement("Action");
		if (oscElement)
		{
			AddOSCArrayMemberCommand *command = new AddOSCArrayMemberCommand(privateActionsArray, privateActions, NULL, "Action", oscBase_, oscElement);
			getProjectGraph()->executeCommand(command);
		}
		privateAction = dynamic_cast<OpenScenario::oscPrivateAction *>(oscElement->getObject());
	}

	return privateAction;

}


//################//
// MOUSE & KEY    //
//################//

/*! \brief .
*
*/
void
OpenScenarioEditor::mouseAction(MouseAction *mouseAction)
{

    QGraphicsSceneMouseEvent *mouseEvent = mouseAction->getEvent();
    ProjectEditor::mouseAction(mouseAction);

    // SELECT //
    //
	ODD::ToolId currentTool = getCurrentTool();
    if (currentTool == ODD::TSG_SELECT)
    {
        QPointF mousePoint = mouseAction->getEvent()->scenePos();

        if (mouseAction->getMouseActionType() == MouseAction::ATM_PRESS)
        {
            if (mouseEvent->button() == Qt::LeftButton)
            {
  
            }
        }

    }
	else if (currentTool == ODD::TOS_ELEMENT)
	{
		QPointF mousePoint = mouseAction->getEvent()->scenePos();

        if (mouseAction->getMouseActionType() == MouseAction::ATM_PRESS)
		{
			if (mouseEvent->button() == Qt::LeftButton)
			{
				QList<QGraphicsItem *> underMouseItems = getTopviewGraph()->getScene()->items(mousePoint);

				if (underMouseItems.count() == 0)		// find the closest road //
				{
					double s;
					double t;
					QVector2D vec;

					RSystemElementRoad * road = getProjectData()->getRoadSystem()->findClosestRoad(mousePoint, s, t, vec);
					if (road)
					{
						// Create new object //
						OpenScenario::oscObjectBase *selectedObject = NULL;
						std::string selectedObjectName;

						QList<DataElement *> elements = getProjectData()->getSelectedElements();
						for (int i = 0; i < elements.size(); i++)
						{
							OSCElement *element = dynamic_cast<OSCElement *>(elements.at(i));
							if (element)
							{
								OpenScenario::oscObjectBase * objectBase = element->getObject();
								if (objectBase)				// all catalog elements have a name
								{
									OpenScenario::oscMember *name = objectBase->getMember("name");
									if (name)
									{
										OpenScenario::oscStringValue *str = dynamic_cast<OpenScenario::oscStringValue *>(name->getValue());
										selectedObjectName = str->getValue();
										selectedObject = oscCatalog_->getCatalogObject(selectedObjectName);
										if (selectedObject)
										{
											break;
										}
									}
								}
							}
						}

						OpenScenario::oscPrivateAction *privateAction = getOrCreatePrivateAction(selectedObjectName);

						if (selectedObject)
						{


							OpenScenario::oscEntities *entitiesObject = openScenarioBase_->Entities.getOrCreateObject();
							OpenScenario::oscMember *objectsMember = entitiesObject->getMember("objects");
							OpenScenario::oscObjectBase *objects = objectsMember->getOrCreateObject();

							OpenScenario::oscArrayMember *oscObjectArray = dynamic_cast<OpenScenario::oscArrayMember *>(objectsMember);

							OSCElement *oscElement = new OSCElement("Object");
							if (oscElement)
							{
								AddOSCArrayMemberCommand *command = new AddOSCArrayMemberCommand(oscObjectArray, objects, NULL, "Object", oscBase_, oscElement);
								getProjectGraph()->executeCommand(command);

								OpenScenario::oscObject *oscObject = static_cast<OpenScenario::oscObject *>(oscElement->getObject());
								OpenScenario::oscCatalogReference *catalogReference = oscObject->CatalogReference.getOrCreateObject();
								catalogReference->name.setValue(selectedObjectName);
								catalogReference->catalog.setValue(oscCatalog_->getCatalogName());

								translateObject(privateAction, road->getID(), s, t);

								OSCItem *oscItem = new OSCItem(oscElement, oscBaseItem_, oscObject, oscCatalog_, mousePoint, road->getID());  
							}
						}
					}
				}	
			}
		}
	}

    //	ProjectEditor::mouseAction(mouseAction);
}

//################//
// TOOL           //
//################//

/*! \brief .
*
*/
void
OpenScenarioEditor::toolAction(ToolAction *toolAction)
{
    // Parent //
    //
    ProjectEditor::toolAction(toolAction);

    ODD::ToolId currentTool = getCurrentTool();

	if (currentTool == ODD::TOS_CREATE_CATALOG)
	{
		OpenScenarioEditorToolAction *action = dynamic_cast<OpenScenarioEditorToolAction *>(toolAction);
		if (action)
		{
			// Create new catalog //
			QString objectName = action->getText();
			
			if (objectName != "")
			{
				OpenScenario::oscCatalog *oscCatalog = getCatalog(objectName.toStdString());

				catalogTree_ = getProjectWidget()->addCatalogTree(objectName, oscCatalog);   
				catalogTree_->setOpenScenarioEditor(this);  

			}
		}
	}
	else if (currentTool == ODD::TOS_ELEMENT)
	{
		OpenScenarioEditorToolAction *action = dynamic_cast<OpenScenarioEditorToolAction *>(toolAction);
		if (action && ((currentTool != lastTool_) || (action->getText() != catalogElement_)))
		{
			// Create new object //
			catalogElement_ = action->getText();
		}
	}
	else if (currentTool == ODD::TOS_BASE)
	{
		OpenScenarioEditorToolAction *action = dynamic_cast<OpenScenarioEditorToolAction *>(toolAction);

		if (action)
		{
			QString oscObjectName = action->getText();
	//		if ((currentTool != lastTool_) || (oscObjectName != lastOSCObjectName_))
			{
				OpenScenario::oscMember *member = openScenarioBase_->getMember(oscObjectName.toStdString());
				if (member)
				{
					OpenScenario::oscObjectBase *object = member->getOrCreateObject();

					OSCElement *memberElement = oscBase_->getOrCreateOSCElement(object);
					if (memberElement)
					{
						// Group undo commands
						//
						getProjectData()->getUndoStack()->beginMacro(QObject::tr("Base Element selected"));

						foreach (DataElement *element, getProjectData()->getSelectedElements())
						{
							DeselectDataElementCommand *command = new DeselectDataElementCommand(element, NULL);
							getProjectGraph()->executeCommand(command); 

						}

						SelectDataElementCommand *command = new SelectDataElementCommand(memberElement, NULL);
						getProjectGraph()->executeCommand(command);		

						getProjectData()->getUndoStack()->endMacro();
					}


					lastOSCObjectName_ = oscObjectName;
				}
			}
		}
	}
    else if (currentTool == ODD::TOS_GRAPHELEMENT)
    {
        OpenScenarioEditorToolAction *action = dynamic_cast<OpenScenarioEditorToolAction *>(toolAction);
        if (action)
        {
            if  (!action->getState())
            {
                if (trajectoryElement_)
                {
                    OpenScenario::oscTrajectory *trajectory = dynamic_cast<OpenScenario::oscTrajectory *>(trajectoryElement_->getObject());
                    createWaypoints(trajectory, getTopviewGraph()->getView()->getSplineControlPoints());
                }
                else
                {
                    addGraphToObserver(getTopviewGraph()->getView()->getSplineControlPoints());
                }

                trajectoryElement_ = NULL;
            }
            else
            {
                QList<DataElement *> elements = getProjectData()->getSelectedElements();
                for (int i = 0; i < elements.size(); i++)
                {
                    OSCElement *element = dynamic_cast<OSCElement *>(elements.at(i));
                    if (element)
                    {
                        OpenScenario::oscTrajectory *trajectory = dynamic_cast<OpenScenario::oscTrajectory *>(element->getObject());
                        if (trajectory)
                        {
                            trajectoryElement_ = element;
                            HideDataElementCommand *command = new HideDataElementCommand(trajectoryElement_);
                            getProjectGraph()->executeCommand(command);
                        }
                    }
                }
            }
        }
  /*      QList<DataElement *> elements = getProjectData()->getSelectedElements();
        for (int i = 0; i < elements.size(); i++)
        {
            OSCElement *element = dynamic_cast<OSCElement *>(elements.at(i));
            if (element && (element != routingElement_))
            {
                OpenScenario::oscRouting *routing = dynamic_cast<OpenScenario::oscRouting *>(element->getObject());
                if (routing)
                {
                    routingElement_ = element;
                    new OSCShapeItem(element, oscBaseItem_);
                }
            }
        } */
    }
	else
	{		
		if (currentTool == ODD::TOS_SAVE_CATALOG)
		{
			// Save catalog //
			//			if (catalog_ && mainWindow_->getActiveProject()->saveCatalogAs())
			/*if (mainWindow_->getActiveProject()->saveAs())
			{
			printStatusBarMsg(tr("File has been saved."), 2000);
			} */


			OpenScenarioEditorToolAction *action = dynamic_cast<OpenScenarioEditorToolAction *>(toolAction);
			if (action)
			{
				// Save catalog //
				oscCatalog_->writeCatalogToDOM();
				oscCatalog_->writeCatalogToDisk();
				oscCatalog_->clearDOM();


/*				QString type = action->getText();

				if (type != "")
				{
					oscCatalog *catalogMember = dynamic_cast<oscCatalog *>(oscCatalog_->getOwnMember());
					if (catalogMember->getName().compare(type.toStdString()) != 0)
					{
						OSCElement *oscElement = getCatalog(type.toStdString());
						oscCatalog_ = oscElement->getObject();
					}
					OpenScenario::OpenScenarioBase *openScenarioBase = openScenarioBase_();
	OpenScenario::oscObjectBase *objectCatalog = openScenarioBase->getMember("catalogs")->getObject();
	objectCatalog = objectCatalog->getMember("objectCatalog")->getObject(); */
	
/*	dynamic_cast<OpenScenario::oscCatalog*>(objectCatalog->getMember(type.toStdString()))->writeCatalogToDOM();
				} */
			}
		}



		/*        if (currentTool == ODD::TSG_SELECT)
		{
		if ((lastTool_ = ODD::TSG_ADD_CONTROL_ENTRY) || (lastTool_ = ODD::TSG_REMOVE_CONTROL_ENTRY))
		{
		foreach (QGraphicsItem *item, getTopviewGraph()->getScene()->items())
		{
		SignalItem * signalItem = dynamic_cast<SignalItem *>(item);
		if (signalItem)
		{
		signalItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
		}
		}
		}
		}
		else if ((currentTool == ODD::TSG_SIGNAL) || (currentTool == ODD::TSG_OBJECT) 
		|| (currentTool == ODD::TSG_BRIDGE) || (currentTool = ODD::TSG_TUNNEL))
		{
		foreach (QGraphicsItem *item, getTopviewGraph()->getScene()->selectedItems())
		{
		item->setSelected(false);
		}
		// does nothing //
		// Problem: The ToolAction is resent, after a warning message has been clicked away. (Due to resend on getting the
		}
		else if (currentTool == ODD::TSG_CONTROLLER)
		{
		QList<ControlEntry *>controlEntryList;
		RSystemElementController *newController = new RSystemElementController("unnamed", "", 0,"", 0.0, controlEntryList);
		AddControllerCommand *command = new AddControllerCommand(newController, getProjectData()->getRoadSystem(), NULL);

		getProjectGraph()->executeCommand(command);
		}
		else if ((currentTool == ODD::TSG_ADD_CONTROL_ENTRY) || (currentTool == ODD::TSG_REMOVE_CONTROL_ENTRY))
		{

		foreach (QGraphicsItem *item, getTopviewGraph()->getScene()->items())
		{
		SignalItem * signalItem = dynamic_cast<SignalItem *>(item);
		if (signalItem)
		{
		signalItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
		}
		}
		}
		else if (currentTool == ODD::TSG_DEL)
		{
		QList<DataElement *>selectedElements = getProjectData()->getSelectedElements();
		QList<RSystemElementController *>selectedControllers;
		foreach (DataElement *element, selectedElements)
		{
		RSystemElementController * controller = dynamic_cast<RSystemElementController *>(element);
		if (controller)
		{
		selectedControllers.append(controller);
		}
		}
		if (selectedControllers.size() > 0)
		{
		// Macro Command //
		//
		int numberOfSelectedControllers = selectedControllers.size();
		if(numberOfSelectedControllers > 1)
		{
		getProjectData()->getUndoStack()->beginMacro(QObject::tr("Set Road Type"));
		}
		for (int i = 0; i < selectedControllers.size(); i++)
		{
		RemoveControllerCommand * delControllerCommand = new RemoveControllerCommand(selectedControllers.at(i), selectedControllers.at(i)->getRoadSystem());
		getProjectGraph()->executeCommand(delControllerCommand);
		}

		// Macro Command //
		//
		if (numberOfSelectedControllers > 1)
		{
		getProjectData()->getUndoStack()->endMacro();
		}
		}*/
	}

	lastTool_ = currentTool;
}
