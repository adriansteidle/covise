/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef CO_TUI_BUTTON_H
#define CO_TUI_BUTTON_H

#include <QObject>

#include "TUIElement.h"

/** Basic Container
 * This class provides basic functionality and a
 * common interface to all Container elements.<BR>
 * The functionality implemented in this class represents a container
 * which arranges its children on top of each other.
 */
class TUIButton : public QObject, public TUIElement
{
    Q_OBJECT

public:
    TUIButton(int id, int type, QWidget *w, int parent, QString name);
    virtual ~TUIButton();
    virtual void setSize(int w, int h) override;
    virtual void setLabel(QString textl) override;

    /// get the Element's classname
    virtual const char *getClassName() const override;

public slots:

    void pressed();
    void released();

protected:
};
#endif
