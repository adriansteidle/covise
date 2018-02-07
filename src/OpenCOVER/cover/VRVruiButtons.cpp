/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include "VRVruiButtons.h"
#include <cover/coVRPluginSupport.h>

#include <OpenVRUI/sginterface/vruiButtons.h>
#include <OpenVRUI/util/vruiLog.h>

using namespace opencover;
using namespace covise;

VRVruiButtons::VRVruiButtons(coPointerButton *button)
: m_button(button)
{
}

VRVruiButtons::~VRVruiButtons()
{
}

coPointerButton *VRVruiButtons::button() const
{
    if (m_button)
        return m_button;
    return cover->getPointerButton();
}

unsigned int VRVruiButtons::wasPressed(unsigned int buttons) const
{
    return button()->wasPressed(buttons);
}

unsigned int VRVruiButtons::wasReleased(unsigned int buttons) const
{
    return button()->wasReleased(buttons);
}

unsigned int VRVruiButtons::getStatus() const
{
    return button()->getState();
}

unsigned int VRVruiButtons::getOldStatus() const
{
    return button()->oldState();
}

int VRVruiButtons::getWheelCount(size_t idx) const
{
    return button()->getWheel(idx);
}
