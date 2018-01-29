#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <string>
#include <set>
#include "Owner.h"
#include "ShortcutListener.h"

namespace covise {
class TokenBuffer;
}

namespace opencover {
namespace ui {

/** \mainpage COVER Abstract UI

  The classes deriving from Element implement the policy and serve as a description language for
  concrete implementations of the user interface for OpenCOVER.

  They are based on Owner and thus can manage the life time of child objects (i.e. delete them
  whenever they themselves are destroyed) and can be managed by parent Owner objects.

  An instance of Manager keeps track of all \ref Element "elements" of the user interface and also
  handles several \ref View "views" onto the user interface. Additional views can be attached and
  removed at any time.

  A View serves two purposes:
  - showing the current state of the application,
  - enabling the interaction with the application.

  Changes from one view are automatically reflected in every other view.
  Concrete implementations of views have to derive from the abstract class View.
  Currently, there are implementation for the menu bar in a Qt window (QtView) and
  for the OpenCOVER VR user interface (VruiView).
  State changes within \ref Element "elements" are communicated to the application by
  invoking appropriate callback function objects.
*/

class Container;
class Group;
class View;

//! base class for UI elements: everything visible to the user derives from this class

/** \note QWidget */
class COVER_UI_EXPORT Element: public Owner, public ShortcutListener {
    friend class Manager;
    friend class Group;
    friend class Container;
 public:
    enum Priority
    {
        Invisible,
        Low,
        Default,
        Medium,
        Toolbar,
        High,
    };
    typedef uint32_t UpdateMaskType;
    enum UpdateMask: UpdateMaskType
    {
        UpdateNothing = 0,
        UpdateVisible = 1,
        UpdateEnabled = 2,
        UpdateText = 4,
        UpdateParent = 8,
        UpdateAll = ~UpdateMaskType(0)
    };
    //! construct as top-level item, life time managed by owner
    /** all derived classes provide a constructor taking the same kind of arguments */
    Element(const std::string &name, Owner *owner);
    //! construct and add to group, life time managed by group
    /** all derived classes provide a constructor taking the same kind of arguments */
    Element(Group *group, const std::string &name);

    //! remove from UI and parents
    virtual ~Element();

    //! unique element id
    int elementId() const;

    //! set importance of Element
    void setPriority(Priority prio);
    //! retrieve importance
    Priority priority() const;

    //! set icon for Element
    void setIcon(const std::string &iconName);
    //! get icon name
    const std::string &iconName() const;

    //! item this Element is a child of
    Group *parent() const;

    std::set<Container *> containers();
    //! request that graphical representation in all views is updated
    virtual void update(UpdateMaskType updateMask=UpdateAll) const;

    //! return text label of this item
    const std::string &text() const;
    //! set text label of this item
    void setText(const std::string &text);

    //! return whether item should be visible in a View
    bool visible(const View *view=nullptr) const;
    //! set item visibility
    void setVisible(bool flag, int viewBits=~0);
    //! return whether item is enabled
    bool enabled() const;
    //! enable or disable item
    void setEnabled(bool flag);

    //! trigger user callback attached to this item
    void trigger() const;

 protected:
    //! reimplement in derived class for calling user callback
    virtual void triggerImplementation() const;
    //! reimplement in derived class for serialization, also call base class
    virtual void save(covise::TokenBuffer &buf) const;
    //! reimplement in derived class for deserialization, also call base class
    virtual void load(covise::TokenBuffer &buf);
    Group *m_parent = nullptr;
    std::set<Container *> m_containers;
    std::string m_label;
    bool m_visible = true;
    bool m_enabled = true;
    Priority m_priority = Default;
    std::string m_iconName;
    int m_viewBits = ~0;
 private:
    mutable int m_id = -1, m_order = -1; // initialized by Manager
};

}
}
#endif
