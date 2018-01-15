/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef CO_TUI_LABEL_H
#define CO_TUI_LABEL_H

#include "TUIElement.h"

class QLabel;
class QPixmap;

/** Basic Container
 * This class provides basic functionality and a
 * common interface to all Container elements.<BR>
 * The functionality implemented in this class represents a container
 * which arranges its children on top of each other.
 */
class TUILabel : public TUIElement
{
private:
public:
    TUILabel(int id, int type, QWidget *w, int parent, QString name);
    virtual ~TUILabel();

    /// get the Element's classname
    virtual const char *getClassName() const override;
    virtual void setLabel(QString text) override;
    virtual void setPixmap(const QPixmap &pm);

protected:
    QLabel *l;
};
#endif
