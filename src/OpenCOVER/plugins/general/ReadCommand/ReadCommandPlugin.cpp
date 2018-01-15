/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/****************************************************************************\
 **                                                            (C)2001 HLRS  **
 **                                                                          **
 ** Description: ReadCommand Plugin (does nothing)                              **
 **                                                                          **
 **                                                                          **
 ** Author: U.Woessner		                                                **
 **                                                                          **
 ** History:  								                                **
 ** Nov-01  v1	    				       		                            **
 **                                                                          **
 **                                                                          **
\****************************************************************************/

#include "ReadCommandPlugin.h"
#include <cover/coVRPluginSupport.h>
#include <cover/OpenCOVER.h>
#include <cover/coVRFileManager.h>
#include <cover/coVRMSController.h>
#include <cover/coVRNavigationManager.h>
#include <cover/VRSceneGraph.h>

#include <QTextStream>

ReadCommandPlugin::ReadCommandPlugin()
    : keepRunning(true)
{
    start();
}

// this is called if the plugin is removed at runtime
ReadCommandPlugin::~ReadCommandPlugin()
{
    keepRunning = false;
    this->wait();
}

void ReadCommandPlugin::run()
{
    static QTextStream istream(stdin, QIODevice::ReadOnly);
    static QTextStream ostream(stdout, QIODevice::WriteOnly);

    QString input;

    while (keepRunning)
    {
        input = istream.readLine();
        if (!input.isNull())
        {
            lock.lock();
            ostream << "ReadCommandPlugin::preFrame info: got command \"" << input << "\"" << endl;
            queue.append(input);
            lock.unlock();
        }
    }
}

void ReadCommandPlugin::preFrame()
{
    lock.lock();
    while (!queue.empty())
    {
        auto navi = coVRNavigationManager::instance();
        QString command = queue.takeFirst();
        if (command == "quit")
        {
            OpenCOVER::instance()->requestQuit();
        }
        else if (command.startsWith("load"))
        {
            QString uri = command.section(' ', 1).simplified();
            if (uri.startsWith("file://"))
                uri.remove(QRegExp("^file://"));
            coVRFileManager::instance()->loadFile(uri.toLatin1().data());
        }
        else if (command == "viewall")
        {
            VRSceneGraph::instance()->viewAll();
        }
        else if (command == "resetview")
        {
            VRSceneGraph::instance()->viewAll(true);
        }
        else if (command == "walk")
        {
            navi->setNavMode(coVRNavigationManager::Walk);
        }
        else if (command == "fly")
        {
            navi->setNavMode(coVRNavigationManager::Fly);
        }
        else if (command == "drive")
        {
            navi->setNavMode(coVRNavigationManager::Glide);
        }
        else if (command == "scale")
        {
            navi->setNavMode(coVRNavigationManager::Scale);
        }
        else if (command == "xform")
        {
            navi->setNavMode(coVRNavigationManager::XForm);
        }
        else if (command.startsWith("wireframe"))
        {
            if (command.section(' ', 1).simplified() == "on")
                VRSceneGraph::instance()->setWireframe(VRSceneGraph::Enabled);
            else
                VRSceneGraph::instance()->setWireframe(VRSceneGraph::Disabled);
        }
    }
    lock.unlock();
}

COVERPLUGIN(ReadCommandPlugin)
