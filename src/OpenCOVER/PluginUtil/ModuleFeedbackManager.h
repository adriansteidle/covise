/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef MODULE_FEEDBACK_MANAGER_H_
#define MODULE_FEEDBACK_MANAGER_H_

#include <util/coTypes.h>

#include <string>

#include <cover/ui/Owner.h>
namespace opencover {
namespace ui {
class Menu;
class Action;
class Button;
}
}

class cp3dplane;

namespace opencover
{
class RenderObject;
class ColorBar;
class coInteractor;
}

#include <osg/Node>
#include <osg/Geode>
#include <osg/MatrixTransform>

// this class manages the interaction (menu and execution)
// of a module (but not the icon)

namespace opencover
{

class PLUGIN_UTILEXPORT ModuleFeedbackManager
        : public ui::Owner
{
public:
    ModuleFeedbackManager(const opencover::RenderObject *, opencover::coInteractor *, const char *pluginName);
    ModuleFeedbackManager(const RenderObject *, const RenderObject *, const char *pluginName);
    virtual ~ModuleFeedbackManager();

    // returns true if inter comes from the same module: used in
    // ModuleFeedbackPlugin::add for adding or updating an entry in
    // ModuleFeedbackPlugin::_ComplexModuleList
    bool compare(coInteractor *inter);
    // returns true if name is an object name from the same module:
    // used in ModuleFeedbackPlugin::remove for deleting an entry
    // in ModuleFeedbackPlugin::_ComplexModuleList
    bool compare(const char *name);
    // returns true if this moduelFeedbackManager is for the same plugin
    bool comparePlugin(const char *pluginName);

    // called for menu update when a new object is received
    virtual void update(const RenderObject *container, coInteractor *);
    virtual void update(const RenderObject *container, const RenderObject *);

    // empty implementation
    virtual void preFrame();

    // empty implementation of 3DTex functionality
    virtual void update3DTex(std::string, cp3dplane *, const char *cmName);

    std::string ModuleName(const char *) const;
    std::string ModuleName()
    {
        return moduleName_;
    };

    // set checkbox and and hide geometry
    void setHideFromGui(bool);

    // set the case name and put item into this menu
    void setCaseFromGui(const char *casename);

    // set the a new name
    void setNameFromGui(const char *casename);

    // show/hide geometry
    virtual void hideGeometry(bool);

    //Transform geometry
    void setMatrix(float *row0, float *row1, float *row2, float *row3);

    //Transform geometry
    void addNodeToCase(osg::Node *node);

    //enable feed back to Colors module
    void addColorbarInteractor(coInteractor *i);

    // do hide button functionality
    virtual void triggerHide(bool state);

    opencover::coInteractor *getInteractor()
    {
        return inter_;
    }
    bool getSyncState();

protected:
    // helper for constructor
    void createMenu();
    void registerObjAtUi(std::string name);

    // helper for update
    void updateMenuNames();
    void updateColorBar(const RenderObject *container);

    // gets either from attribute OBJECTNAME or from the module name
    // a suggestion for the menu name, the result is kept in _menuName
    //std::string suggestMenuName();
    std::string getMenuName() const;

    ui::Menu *menu_ = nullptr; // the menu for the interaction of the module managed by the instance of this class
    ui::Button *hideCheckbox_ = nullptr; // hide geometry
    ui::Button *syncCheckbox_ = nullptr; // sync interaction
    ui::Action *newButton_ = nullptr; // copy this module
    ui::Action *deleteButton_ = nullptr; // delete this module
    ui::Button *executeCheckbox_ = nullptr; // execute module button
    //ui::coSubMenuItem *colorsButton_; // open colorbar
    opencover::coInteractor *inter_ = nullptr; // the last interaction got from the module at issue
    bool inExecute_; // switch checkbox off when new object arrived
    std::string menuName_; // name associated to _menuItem: its updated when a new object is received

    // Overload this function if you want to do anything before an EXEC
    // call is sent to a module. Always call it before doing an EXEC call!
    virtual void preExecCB(opencover::coInteractor *){}

    // search the corresponding node in sg
    osg::Node *findMyNode();
    std::vector<osg::Geode *> findMyGeode();

    std::string geomObjectName_;
    std::string containerObjectName_;
    std::string attrObjectName_;
    std::string attrPartName_;

    std::string initialObjectName_; //we have to save it for the grmsg, because _inter is not always valid
    osg::ref_ptr<osg::MatrixTransform> geometryCaseDCS_;

private:
    // it is used in ModuleFeedbackManager::compare(const char *name)
    std::string moduleName_;

    std::string pName_;

    ui::Menu *coviseMenu_ = nullptr;
    ui::Menu *parentMenu_ = nullptr;
    ui::Menu *caseMenu_ = nullptr;
    std::string caseName_;

    std::string visItemName_; //"Tracer_1..."
    std::string visMenuName_; // "Tracer_1"
    void sendHideMsg(bool);
    std::vector<osg::Geode *> findRecMyGeode(osg::Node *node);
    osg::ref_ptr<osg::Node> myNode_;
    osg::ref_ptr<osg::Group> myNodesParent_;
};
}
#endif
