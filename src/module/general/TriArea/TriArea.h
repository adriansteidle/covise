/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef _TRIAREA_H
#define _TRIAREA_H
/**************************************************************************\ 
 **                                                           (C)1997 RUS  **
 **                                                                        **
 ** Description: Generate Normals for Polygonal Data	                  **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 ** Author:                                                                **
 **                                                                        **
 **                            Uwe Woessner                                **
 **                Computer Center University of Stuttgart                 **
 **                            Allmandring 30                              **
 **                            70550 Stuttgart                             **
 **                                                                        **
 ** Date:  25.08.97  V0.1                                                  **
\**************************************************************************/

#include <api/coSimpleModule.h>
using namespace covise;

#include <util/coviseCompat.h>

// Jetzt brauchen wir die Klasse Covise_Set_Handler nicht mehr

class TriArea : public coSimpleModule
{
    static const char *s_defMapNames[];
    coChoiceParam *p_areamode;
    enum AreaSelectMap
    {
        TriangleArea = 0,
        RatioMinToMaxSide = 1,
        AngleNormalToCamera = 2,
        SomeThingElse = 3
    } areamode;

private:
    virtual int compute(const char *port);

    ////////// ports
    coInputPort *p_inPort;
    coOutputPort *p_outPort;
    coFloatVectorParam *p_cameraPosition; 

    //  Shared memory data
    coDistributedObject *tri_area(const coDistributedObject *m, float *cameraPosition, const char *objName);

public:
    TriArea(int argc, char *argv[]);
};
#endif // _TRIAREA_H
