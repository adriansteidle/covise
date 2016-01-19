/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */

#ifndef OSC_INIT_DYNAMICS_H
#define OSC_INIT_DYNAMICS_H

#include <oscExport.h>
#include <oscObjectBase.h>
#include <oscObjectVariable.h>

#include <oscVariables.h>


namespace OpenScenario {

/// \class This class represents a generic OpenScenario Object
class OPENSCENARIOEXPORT oscInitDynamics: public oscObjectBase
{
public:
    oscInitDynamics()
    {
        OSC_ADD_MEMBER(velocity);
        OSC_ADD_MEMBER(acceleration);
    };

    oscDouble velocity;
    oscDouble acceleration;
};

typedef oscObjectVariable<oscInitDynamics *>oscInitDynamicsMember;

}

#endif //OSC_INIT_DYNAMICS_H
