/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include <assert.h>
#include <stdio.h>

#include <QFrame>
#include <QTabWidget>
#include <QGridLayout>
#include <QScrollArea>

#include "TUITab.h"
#include "TUITabFolder.h"
#include "TUIApplication.h"
#if !defined _WIN32_WCE && !defined ANDROID_TUI
#include <net/tokenbuffer.h>
#else
#include <wce_msg.h>
#endif

#include <iostream>

/// Constructor
TUITab::TUITab(int id, int type, QWidget *w, int parent, QString name)
    : TUIContainer(id, type, w, parent, name)
{
    label = name;
    QScrollArea *scroll = nullptr;

    bool inMainFolder = parent==3;
    auto parentWidget = w;
    if (inMainFolder)
    {
        scroll = new QScrollArea(w);
        scroll->setMinimumWidth(300);
        scroll->setMinimumHeight(300);
        scroll->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
        scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scroll->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
        parentWidget = scroll;
    }

    QFrame *frame = new QFrame(parentWidget);
    frame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    frame->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    frame->setContentsMargins(0, 0, 0, 0);
    widget = frame;
    layout = new QGridLayout(frame);
    frame->setLayout(layout);
    if (scroll)
    {
        scroll->setWidget(frame);
        scroll->setWidgetResizable(true);
        widget = scroll;
    }

    firstTime = true;
}

/// Destructor
TUITab::~TUITab()
{
    removeAllChildren();
    delete layout;
    layout = nullptr;
    delete widget;
    widget = nullptr;
}

void TUITab::activated()
{

    if (!firstTime)
    {
        covise::TokenBuffer tb;
        tb << ID;
        tb << TABLET_ACTIVATED;
        TUIMainWindow::getInstance()->send(tb);
    }
    firstTime = false;
}

void TUITab::setLabel(QString textl)
{
    TUIContainer::setLabel(textl);
    if (TUIContainer* p = getParent())
    {
        p->addElementToLayout(this);
    }
}

void TUITab::deActivate(TUITab *activedTab)
{

    if (activedTab != this)
    {
        covise::TokenBuffer tb;
        tb << ID;
        tb << TABLET_DISACTIVATED;
        TUIMainWindow::getInstance()->send(tb);
    }
}

void TUITab::setPos(int x, int y)
{
    xPos = x;
    yPos = y;
    TUIContainer *parent = getParent();
    if (parent)
    {
        parent->addElementToLayout(this);
        if (auto folder = dynamic_cast<TUITabFolder *>(parent))
            folder->setCurrentIndex(folder->indexOf(widget));
        else
            std::cerr << "error: parent is not a TUITabFolder" << std::endl;
    }
    else
    {
        TUIMainWindow::getInstance()->addElementToLayout(this);
    }
    widget->setVisible(!hidden);
}

void TUITab::setValue(TabletValue type, covise::TokenBuffer &tb)
{
    if (type == TABLET_BOOL)
    {
        if (auto parent = getParent())
        {
            if (auto folder = dynamic_cast<TUITabFolder *>(parent))
                folder->setCurrentIndex(folder->indexOf(widget));
            else
                std::cerr << "error: parent is not a TUITabFolder" << std::endl;
        }
    }
    TUIContainer::setValue(type, tb);
}

void TUITab::setHidden(bool hide)
{
    TUIContainer::setHidden(hide);
    if (TUIContainer *parent = getParent())
    {
        if (auto folder = dynamic_cast<TUITabFolder *>(parent))
        {
            int index = folder->indexOf(widget);
            if (hidden)
            {
                if (index >= 0)
                    folder->removeTab(index);
            }
            else
            {
                if (index < 0)
                    folder->addTab(widget, label);
            }
        }
        else
        {
            std::cerr << "TUITab::setHidden(): parent of " << getID() << "/" << getName().toStdString() << " is not a TUITabFolder but a " << parent->getClassName() << std::endl;
        }
    }
}

const char *TUITab::getClassName() const
{
    return "TUITab";
}
