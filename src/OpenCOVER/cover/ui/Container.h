#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include <memory>
#include <vector>
#include "Export.h"

namespace opencover {
namespace ui {

class Element;

//! mix-in class for containers of UI \ref Element "elements"
class COVER_UI_EXPORT Container {
 public:
    enum Position {
        Front = -1,
        Back = -2,
    };
    virtual ~Container();

    virtual bool add(Element *elem, int where=Back);
    virtual bool remove(Element *elem);

    size_t numChildren() const;
    Element *child(size_t index) const;

 protected:
    std::vector<Element *> m_children;
    void clearChildren();
};

}
}
#endif
