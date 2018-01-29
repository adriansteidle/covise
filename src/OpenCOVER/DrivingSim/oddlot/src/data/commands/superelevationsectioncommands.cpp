/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   16.07.2010
**
**************************************************************************/

#include "superelevationsectioncommands.hpp"

// Data //
//
#include "src/data/roadsystem/rsystemelementroad.hpp"
#include "src/data/roadsystem/sections/superelevationsection.hpp"
#include "src/data/roadsystem/sections/crossfallsection.hpp"

#include "src/data/scenerysystem/heightmap.hpp"
#include "src/cover/coverconnection.hpp"

// Utils //
//
#include "math.h"

#define MIN_SUPERELEVATIONSECTION_LENGTH 1.0

//#######################//
// SplitSuperelevationSectionCommand //
//#######################//

SplitSuperelevationSectionCommand::SplitSuperelevationSectionCommand(SuperelevationSection *superelevationSection, double splitPos, DataCommand *parent)
    : DataCommand(parent)
    , oldSection_(superelevationSection)
    , newSection_(NULL)
    , splitPos_(splitPos)
{
    // Check for validity //
    //
    if ((oldSection_->getDegree() > 1) // only lines allowed
        || (fabs(splitPos_ - oldSection_->getSStart()) < MIN_SUPERELEVATIONSECTION_LENGTH)
        || (fabs(oldSection_->getSEnd() - splitPos_) < MIN_SUPERELEVATIONSECTION_LENGTH) // minimum length 1.0 m
        )
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Split SuperelevationSection (invalid!)"));
        return;
    }
    else
    {
        setValid();
        setText(QObject::tr("Split SuperelevationSection"));
    }

    // New section //
    //
    newSection_ = new SuperelevationSection(splitPos, oldSection_->getSuperelevationDegrees(splitPos), oldSection_->getB(), 0.0, 0.0);
}

/*! \brief .
*
*/
SplitSuperelevationSectionCommand::~SplitSuperelevationSectionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        delete newSection_;
    }
    else
    {
        // nothing to be done
        // the section is now owned by the road
    }
}

/*! \brief .
*
*/
void
SplitSuperelevationSectionCommand::redo()
{
    //  //
    //
    oldSection_->getParentRoad()->addSuperelevationSection(newSection_);

    setRedone();
}

/*! \brief .
*
*/
void
SplitSuperelevationSectionCommand::undo()
{
    //  //
    //
    newSection_->getParentRoad()->delSuperelevationSection(newSection_);

    setUndone();
}

//#######################//
// MergeSuperelevationSectionCommand //
//#######################//

MergeSuperelevationSectionCommand::MergeSuperelevationSectionCommand(SuperelevationSection *superelevationSectionLow, SuperelevationSection *superelevationSectionHigh, DataCommand *parent)
    : DataCommand(parent)
    , oldSectionLow_(superelevationSectionLow)
    , oldSectionHigh_(superelevationSectionHigh)
    , newSection_(NULL)
{
    parentRoad_ = superelevationSectionLow->getParentRoad();

    // Check for validity //
    //
    if (/*(oldSectionLow_->getDegree() > 1)
        || (oldSectionHigh_->getDegree() > 1) // only lines allowed
        ||*/ (parentRoad_ != superelevationSectionHigh->getParentRoad()) // not the same parents
        || superelevationSectionHigh != parentRoad_->getSuperelevationSection(superelevationSectionLow->getSEnd()) // not consecutive
        )
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Merge SuperelevationSection (invalid!)"));
        return;
    }
    else
    {
        setValid();
        setText(QObject::tr("Merge SuperelevationSection"));
    }

    // New section //
    //
    /*double deltaLength = superelevationSectionHigh->getSEnd() - superelevationSectionLow->getSStart();
    double deltaHeight = superelevationSectionHigh->getSuperelevationDegrees(superelevationSectionHigh->getSEnd()) - superelevationSectionLow->getSuperelevationDegrees(superelevationSectionLow->getSStart());
    newSection_ = new SuperelevationSection(oldSectionLow_->getSStart(), oldSectionLow_->getA(), deltaHeight / deltaLength, 0.0, 0.0);
    if (oldSectionHigh_->isElementSelected() || oldSectionLow_->isElementSelected())
    {
        newSection_->setElementSelected(true); // keep selection
    }*/
    
    double l = superelevationSectionHigh->getSEnd() - superelevationSectionLow->getSStart();

    double h0 = superelevationSectionLow->getSuperelevationDegrees(superelevationSectionLow->getSStart());
    double dh0 = superelevationSectionLow->getSuperelevationSlopeDegrees(superelevationSectionLow->getSStart());

    double h1 = superelevationSectionHigh->getSuperelevationDegrees(superelevationSectionHigh->getSEnd());
    double dh1 = superelevationSectionHigh->getSuperelevationSlopeDegrees(superelevationSectionHigh->getSEnd());

    double d = (dh1 + dh0 - 2.0 * h1 / l + 2.0 * h0 / l) / (l * l);
    double c = (h1 - d * l * l * l - dh0 * l - h0) / (l * l);
    newSection_ = new SuperelevationSection(oldSectionLow_->getSStart(), h0, dh0, c, d);
    // Done //
    //
    setValid();
    setText(QObject::tr("Merge SuperelevationSection"));
}

/*! \brief .
*
*/
MergeSuperelevationSectionCommand::~MergeSuperelevationSectionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        delete newSection_;
    }
    else
    {
        delete oldSectionLow_;
        delete oldSectionHigh_;
    }
}

/*! \brief .
*
*/
void
MergeSuperelevationSectionCommand::redo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(oldSectionLow_);
    parentRoad_->delSuperelevationSection(oldSectionHigh_);

    parentRoad_->addSuperelevationSection(newSection_);

    setRedone();
}

/*! \brief .
*
*/
void
MergeSuperelevationSectionCommand::undo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(newSection_);

    parentRoad_->addSuperelevationSection(oldSectionLow_);
    parentRoad_->addSuperelevationSection(oldSectionHigh_);

    setUndone();
}

//#######################//
// RemoveSuperelevationSectionCommand //
//#######################//

RemoveSuperelevationSectionCommand::RemoveSuperelevationSectionCommand(SuperelevationSection *superelevationSection, DataCommand *parent)
    : DataCommand(parent)
    , oldSectionLow_(NULL)
    , oldSectionMiddle_(superelevationSection)
    , oldSectionHigh_(NULL)
    , newSectionHigh_(NULL)
{
    parentRoad_ = oldSectionMiddle_->getParentRoad();
    if (!parentRoad_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot remove SuperelevationSection. No ParentRoad."));
        return;
    }

    oldSectionLow_ = parentRoad_->getSuperelevationSectionBefore(oldSectionMiddle_->getSStart());
    if (!oldSectionLow_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot remove SuperelevationSection. No section to the left."));
        return;
    }

    oldSectionHigh_ = parentRoad_->getSuperelevationSection(oldSectionMiddle_->getSEnd());
    if (!oldSectionHigh_ || (oldSectionHigh_ == oldSectionMiddle_))
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot remove SuperelevationSection. No section to the right."));
        return;
    }

    if (oldSectionLow_->getDegree() > 1)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot remove SuperelevationSection. Section to the left is not linear."));
        return;
    }

    if (oldSectionHigh_->getDegree() > 1)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot remove SuperelevationSection. Section to the right is not linear."));
        return;
    }

    // Intersection point //
    //
    double sLow = oldSectionLow_->getSStart();
    double s = 0.0;
    if (fabs(oldSectionLow_->getB() - oldSectionHigh_->getB()) < NUMERICAL_ZERO8)
    {
        // Parallel //
        //
        if (fabs(oldSectionLow_->getA() - oldSectionHigh_->getA()) < NUMERICAL_ZERO8)
        {
            s = 0.5 * (oldSectionLow_->getSEnd() + oldSectionHigh_->getSStart()); // meet half way
        }
        else
        {
            setInvalid(); // Invalid
            setText(QObject::tr("Cannot remove SuperelevationSection. Sections to the left and right do not intersect."));
            return;
        }
    }
    else
    {
        // Not parallel //
        //
        s = sLow + (oldSectionHigh_->getSuperelevationDegrees(sLow) - oldSectionLow_->getSuperelevationDegrees(sLow)) / (oldSectionLow_->getB() - oldSectionHigh_->getB());
        if ((s - sLow < MIN_SUPERELEVATIONSECTION_LENGTH)
            || (oldSectionHigh_->getSEnd() - s < MIN_SUPERELEVATIONSECTION_LENGTH))
        {
            setInvalid(); // Invalid
            setText(QObject::tr("Cannot remove SuperelevationSection. Sections to the left and right do not intersect."));
            return;
        }
    }
    //	qDebug() << "s: " << s << ", sLow: " << sLow;

    // New section //
    //
    newSectionHigh_ = new SuperelevationSection(s, oldSectionLow_->getSuperelevationDegrees(s), oldSectionHigh_->getB(), 0.0, 0.0);
    if (oldSectionHigh_->isElementSelected() || oldSectionMiddle_->isElementSelected())
    {
        newSectionHigh_->setElementSelected(true); // keep selection
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Remove SuperelevationSection"));
}

/*! \brief .
*
*/
RemoveSuperelevationSectionCommand::~RemoveSuperelevationSectionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        delete newSectionHigh_;
    }
    else
    {
        delete oldSectionMiddle_;
        delete oldSectionHigh_;
    }
}

/*! \brief .
*
*/
void
RemoveSuperelevationSectionCommand::redo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(oldSectionMiddle_);
    parentRoad_->delSuperelevationSection(oldSectionHigh_);

    parentRoad_->addSuperelevationSection(newSectionHigh_);

    setRedone();
}

/*! \brief .
*
*/
void
RemoveSuperelevationSectionCommand::undo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(newSectionHigh_);

    parentRoad_->addSuperelevationSection(oldSectionMiddle_);
    parentRoad_->addSuperelevationSection(oldSectionHigh_);

    setUndone();
}

//#######################//
// SmoothSuperelevationSectionCommand //
//#######################//

SmoothSuperelevationSectionCommand::SmoothSuperelevationSectionCommand(SuperelevationSection *superelevationSectionLow, SuperelevationSection *superelevationSectionHigh, double radius, DataCommand *parent)
    : DataCommand(parent)
    , oldSectionLow_(superelevationSectionLow)
    , oldSectionHigh_(superelevationSectionHigh)
    , newSection_(NULL)
    , newSectionHigh_(NULL)
    , radius_(radius)
{
    parentRoad_ = superelevationSectionLow->getParentRoad();

    if (radius_ < 0.01 || radius_ > 1000000.0)
    {
        setInvalid();
        setText(QObject::tr("Cannot smooth SuperelevationSection. Radius not in interval [0.01, 100000.0]."));
        return;
    }

    if (oldSectionLow_->getDegree() > 1)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. Section to the left is not linear."));
        return;
    }

    if (oldSectionHigh_->getDegree() > 1)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. Section to the right is not linear."));
        return;
    }

    if (parentRoad_ != superelevationSectionHigh->getParentRoad()) // not the same parents
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. Section do not belong to the same road."));
        return;
    }

    if (superelevationSectionHigh != parentRoad_->getSuperelevationSection(superelevationSectionLow->getSEnd())) // not consecutive
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. Sections are not consecutive."));
        return;
    }

    // Coordinates //
    //
    double bLow = oldSectionLow_->getB();
    double bHigh = oldSectionHigh_->getB();
    if (fabs(bHigh - bLow) < 0.005)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. Sections are (approximately) parallel."));
        return;
    }

    double b = bLow;
    double c = 1.0 / radius_; // curvature
    double l = (bHigh - b) / (2.0 * c);
    if (l < 0)
    {
        c = -c;
        l = -l;
    }
    if (l < MIN_SUPERELEVATIONSECTION_LENGTH)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. New Section would be too short."));
        return;
    }

    double h = b * l + c * l * l;
    sLow_ = superelevationSectionHigh->getSStart() + (h - bHigh * l) / (bHigh - bLow);
    sHigh_ = l + sLow_;
    //	qDebug() << "sLow_: " << sLow_ << ", sHigh_: " << sHigh_;

    if ((sLow_ < superelevationSectionLow->getSStart() + MIN_SUPERELEVATIONSECTION_LENGTH) // plus one meter
        || (sHigh_ > superelevationSectionHigh->getSEnd() - MIN_SUPERELEVATIONSECTION_LENGTH) // minus one meter
        )
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot smooth SuperelevationSection. New Section would be too long."));
        return;
    }

    // New sections //
    //
    newSection_ = new SuperelevationSection(sLow_, oldSectionLow_->getSuperelevationDegrees(sLow_), b, c, 0.0);
    newSectionHigh_ = new SuperelevationSection(sHigh_, oldSectionHigh_->getSuperelevationDegrees(sHigh_), oldSectionHigh_->getB(), 0.0, 0.0);
    if (oldSectionHigh_->isElementSelected())
    {
        newSectionHigh_->setElementSelected(true);
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Smooth SuperelevationSection"));
}

/*! \brief .
*
*/
SmoothSuperelevationSectionCommand::~SmoothSuperelevationSectionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        delete newSection_;
        delete newSectionHigh_;
    }
    else
    {
        delete oldSectionHigh_;
    }
}

/*! \brief .
*
*/
void
SmoothSuperelevationSectionCommand::redo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(oldSectionHigh_);

    parentRoad_->addSuperelevationSection(newSection_);
    parentRoad_->addSuperelevationSection(newSectionHigh_);

    setRedone();
}

/*! \brief .
*
*/
void
SmoothSuperelevationSectionCommand::undo()
{
    //  //
    //
    parentRoad_->delSuperelevationSection(newSection_);
    parentRoad_->delSuperelevationSection(newSectionHigh_);

    parentRoad_->addSuperelevationSection(oldSectionHigh_);

    setUndone();
}

//##########################//
// SuperelevationMovePointsCommand //
//##########################//

SuperelevationMovePointsCommand::SuperelevationMovePointsCommand(const QList<SuperelevationSection *> &endPointSections, const QList<SuperelevationSection *> &startPointSections, const QPointF &deltaPos, DataCommand *parent)
    : DataCommand(parent)
    , endPointSections_(endPointSections)
    , startPointSections_(startPointSections)
    , superelevationOnly_(false)
    , deltaPos_(deltaPos)
{
    // Check for validity //
    //
    if (fabs(deltaPos_.manhattanLength()) < NUMERICAL_ZERO8 || (endPointSections_.isEmpty() && startPointSections_.isEmpty()))
    {
        setInvalid(); // Invalid because no change.
        //		setText(QObject::tr("Cannot move superelevation point. Nothing to be done."));
        setText("");
        return;
    }

    foreach (SuperelevationSection *section, endPointSections_)
    {
        oldEndPointsBs_.append(section->getB());

        if (fabs(section->getSEnd() - section->getParentRoad()->getLength()) < NUMERICAL_ZERO8) //
        {
            superelevationOnly_ = true;
        }
    }

    bool tooShort = false;
    foreach (SuperelevationSection *section, startPointSections_)
    {
        oldStartPointsAs_.append(section->getA());
        oldStartPointsBs_.append(section->getB());
        oldStartPointsSs_.append(section->getSStart());

        if (fabs(section->getSStart()) < NUMERICAL_ZERO8) // first section of the road
        {
            superelevationOnly_ = true;
        }
        else if ((section->getLength() - deltaPos_.x() < MIN_SUPERELEVATIONSECTION_LENGTH) // min length at end
                 || (section->getParentRoad()->getSuperelevationSectionBefore(section->getSStart())->getLength() + deltaPos_.x() < MIN_SUPERELEVATIONSECTION_LENGTH))
        {
            tooShort = true;
        }
    }

    if (!superelevationOnly_ && tooShort)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("Cannot move superelevation point. A section would be too short."));
        return;
    }

    if (superelevationOnly_)
    {
        deltaPos_.setX(0.0);
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Move Superelevation Point"));
}

/*! \brief .
*
*/
SuperelevationMovePointsCommand::~SuperelevationMovePointsCommand()
{
}

/*! \brief .
*
*/
void
SuperelevationMovePointsCommand::redo()
{
    // Set points //
    //
    int i = 0;
    foreach (SuperelevationSection *section, endPointSections_)
    {
        double startSuperelevation = section->getSuperelevationDegrees(section->getSStart());
        double endSuperelevation = section->getSuperelevationDegrees(section->getSEnd()) + deltaPos_.y();
        double slope = (endSuperelevation - startSuperelevation) / (section->getLength() + deltaPos_.x());
        section->setParametersDegrees(startSuperelevation, slope, 0.0, 0.0);
        ++i;
    }
    i = 0;
    foreach (SuperelevationSection *section, startPointSections_)
    {
        double startSuperelevation = section->getSuperelevationDegrees(section->getSStart()) + deltaPos_.y();
        double endSuperelevation = section->getSuperelevationDegrees(section->getSEnd());
        double slope = (endSuperelevation - startSuperelevation) / (section->getLength() - deltaPos_.x());
        section->setParametersDegrees(startSuperelevation, slope, 0.0, 0.0);
        ++i;
    }

    // Move //
    //
    if (!superelevationOnly_)
    {
        foreach (SuperelevationSection *section, startPointSections_)
        {
            section->getParentRoad()->moveSuperelevationSection(section->getSStart(), section->getSStart() + deltaPos_.x());
        }
    }

    setRedone();
}

/*! \brief
*
*/
void
SuperelevationMovePointsCommand::undo()
{
    // Set points //
    //
    int i = 0;
    foreach (SuperelevationSection *section, endPointSections_)
    {
        section->setParametersDegrees(section->getA(), oldEndPointsBs_[i], 0.0, 0.0);
        ++i;
    }
    i = 0;
    foreach (SuperelevationSection *section, startPointSections_)
    {
        section->setParametersDegrees(oldStartPointsAs_[i], oldStartPointsBs_[i], 0.0, 0.0);
        ++i;
    }

    // Move //
    //
    if (!superelevationOnly_)
    {
        i = 0;
        foreach (SuperelevationSection *section, startPointSections_)
        {
            section->getParentRoad()->moveSuperelevationSection(section->getSStart(), oldStartPointsSs_[i]);
            ++i;
        }
    }

    setUndone();
}

/*! \brief Attempts to merge this command with other. Returns true on success; otherwise returns false.
*
*/
bool
SuperelevationMovePointsCommand::mergeWith(const QUndoCommand *other)
{
    // Check Ids //
    //
    if (other->id() != id())
    {
        return false;
    }

    const SuperelevationMovePointsCommand *command = static_cast<const SuperelevationMovePointsCommand *>(other);

    // Check sections //
    //
    if (endPointSections_.size() != command->endPointSections_.size()
        || startPointSections_.size() != command->startPointSections_.size())
    {
        return false; // not the same amount of sections
    }

    for (int i = 0; i < endPointSections_.size(); ++i)
    {
        if (endPointSections_[i] != command->endPointSections_[i])
        {
            return false; // different sections
        }
    }
    for (int i = 0; i < startPointSections_.size(); ++i)
    {
        if (startPointSections_[i] != command->startPointSections_[i])
        {
            return false; // different sections
        }
    }

    // Success //
    //
    deltaPos_ += command->deltaPos_; // adjust to new pos, then let the undostack kill the new command

    return true;
}

//##########################//
// ApplyHeightMapSuperelevationCommand //
//##########################//

ApplyHeightMapSuperelevationCommand::ApplyHeightMapSuperelevationCommand(RSystemElementRoad *road, const QList<Heightmap *> &maps, double heightOffset, double sampleDistance, double maxDeviation, double lowPassFilter, bool useCubic, bool smoothLinear, double radius, DataCommand *parent)
    : DataCommand(parent)
    , road_(road)
    , maps_(maps)
    , heightOffset_(heightOffset)
    , sampleDistance_(sampleDistance)
    , maxDeviation_(maxDeviation)
    , lowPassFilter_(lowPassFilter)
    , useCubic_(useCubic)
    , smoothLinear_(smoothLinear)
    , radius_(radius)
{
    // Check for validity //
    //
    if (!road || (maps.isEmpty() && !COVERConnection::instance()->isConnected()) || sampleDistance < NUMERICAL_ZERO3 || maxDeviation < NUMERICAL_ZERO3)
    {
        setInvalid(); // Invalid because no change.
        setText("Apply Heightmap: invalid parameters!");
        return;
    }
    

    // Sections //
    //
    oldSections_ = road->getSuperelevationSections();

    // Initialization //
    //
    double sStart = 0.0;
    double sEnd = road_->getLength();
    if (sEnd < sStart)
        sEnd = sStart;

    double pointsPerMeter = 1.0 / sampleDistance;
    int pointCount = int(ceil((sEnd - sStart) * pointsPerMeter));
    if (pointCount < 2)
    {
        // TODO
        pointCount = 2;
        qDebug() << road->getID().speakingName() << " Segment too short: duplicate points per meter";
    }
    double segmentLength = (sEnd - sStart) / (pointCount - 1);

    // Read superelevations //
    //
    double *sampleSuperelevations = new double[pointCount];
    if(COVERConnection::instance()->isConnected())
    {
        covise::TokenBuffer tb;
        tb << MSG_GetHeight;
        tb << (pointCount*2);
        for (int i = 0; i < pointCount; ++i)
        {
            double s = sStart + i * segmentLength; // [sStart, sEnd]
            
            float wr = road_->getMaxWidth(s);
            float wl = road_->getMinWidth(s);
            QPointF posr = road_->getGlobalPoint(s, wr);
            QPointF posl = road_->getGlobalPoint(s, wl);
            tb << (float)posr.x();
            tb << (float)posr.y();
            tb << (float)posl.x();
            tb << (float)posl.y();
        }
        COVERConnection::instance()->send(tb);
        covise::Message *msg=NULL;
        if(COVERConnection::instance()->waitForMessage(&msg))
        {
            covise::TokenBuffer rtb(msg);
            int type;
            rtb >>  type;
            if(type == MSG_GetHeight)
            {
                int pc;
                rtb >> pc;
                if((pc/2) == pointCount)
                {
                    float hr,hl;
                    for (int i = 0; i < pointCount; ++i)
                    {
                        double s = sStart + i * segmentLength; // [sStart, sEnd]

                        float wr = road_->getMaxWidth(s);
                        float wl = road_->getMinWidth(s);
                        rtb >> hr;
                        rtb >> hl;
                        
                        sampleSuperelevations[i] = atan((hr - hl) / (wr-wl))*180.0/M_PI;
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            pointCount =0;
        }
        
    }
    else
    {
        for (int i = 0; i < pointCount; ++i)
        {
            double s = sStart + i * segmentLength; // [sStart, sEnd]
            sampleSuperelevations[i] = getSuperelevation(s);
        }
    }


#if 1
    // Low pass filter //
    //
    double *superelevations = new double[pointCount];
    superelevations[0] = sampleSuperelevations[0];
    double alpha = lowPassFilter_;
    for (int i = 1; i < pointCount - 1; ++i)
    {
        superelevations[i] = 0.5 * (1 - alpha) * sampleSuperelevations[i - 1] + alpha * sampleSuperelevations[i] + 0.5 * (1 - alpha) * sampleSuperelevations[i + 1];
    }
    superelevations[pointCount - 1] = sampleSuperelevations[pointCount - 1];
    delete[] sampleSuperelevations;
#endif

    // Cubic approximation //
    //
	if (useCubic_ && pointCount > 0)
    {
        
        // Calculate Slopes //
        //
        double *dsuperelevations = new double[pointCount];
        dsuperelevations[0] = (superelevations[1] - superelevations[0]) / segmentLength;

        // Create Sections //
        //
        int lastPoint=0;
        SuperelevationSection *oldSection=NULL;
        
        for (int i = 1; i < pointCount; ++i)
        {
            double currentLength = segmentLength*(i-lastPoint);
            if(i < pointCount - 1)
            {
                dsuperelevations[i] = 0.5 * (superelevations[i] - superelevations[lastPoint]) / (segmentLength*(i-lastPoint)) + 0.5 * (superelevations[i + 1] - superelevations[i]) / segmentLength;
                // evtl. gewichten
            }
            else
            {
                dsuperelevations[pointCount - 1] = (superelevations[pointCount - 1] - superelevations[lastPoint]) / (segmentLength*((pointCount - 1)-lastPoint));
            }
            double s = sStart + lastPoint * segmentLength; // [sStart, sEnd]

            double d = superelevations[lastPoint];
            double c = dsuperelevations[lastPoint];
            double a = (dsuperelevations[i] + c - 2.0 * superelevations[i] / currentLength + 2.0 * d / currentLength) / (currentLength * currentLength);
            double b = (superelevations[i] - a * currentLength * currentLength * currentLength - c * currentLength - d) / (currentLength * currentLength);

            double maxDistance=0;
            
            SuperelevationSection *section = new SuperelevationSection(s, d, c, b, a);
            for(int n=(lastPoint+1);n<i;n++)
            {
                double dist = fabs(superelevations[n] - section->getSuperelevationDegrees(n*segmentLength));
                if(dist > maxDistance)
                    maxDistance = dist;
            }
            if(maxDistance < maxDeviation_)
            {
                delete oldSection;
                oldSection = section;
            }
            else
            {
                newSections_.insert(s, oldSection);
                lastPoint = i-1;
                
                double d = superelevations[i-1];
                double c = dsuperelevations[i-1];
                double a = (dsuperelevations[i] + c - 2.0 * superelevations[i] / segmentLength + 2.0 * d / segmentLength) / (segmentLength * segmentLength);
                double b = (superelevations[i] - a * segmentLength * segmentLength * segmentLength - c * segmentLength - d) / (segmentLength * segmentLength);
                double s = sStart + (i-1) * segmentLength; // [sStart, sEnd]
                oldSection = new SuperelevationSection(s, d, c, b, a);
                lastPoint = i-1;
            }

        }
        delete[] dsuperelevations;
        if(oldSection)
        {
            double s = sStart + lastPoint * segmentLength;
            newSections_.insert(s, oldSection);
        }

    }

    // Linear approximation //
    //
	else if (pointCount > 0)
    {
        // Create Sections //
        //

        int lastIndex = 0;
        double sectionStart = sStart;
        for (int i = 2; i < pointCount; ++i)
        {
            double slope = (superelevations[i] - superelevations[lastIndex]) / ((i - lastIndex) * segmentLength);

            int j;
            for (j = i - 1; j > lastIndex; j--)
            {
                double predictedHeight = superelevations[lastIndex] + slope * (sStart + j * segmentLength);
                if (fabs(predictedHeight - superelevations[j]) > maxDeviation) // take the last one
                {
                    break;
                }
            }

            if ((i != pointCount - 1) && (j == lastIndex))
            {
                continue;
            }

            slope = (superelevations[i - 1] - superelevations[lastIndex]) / (((i - 1) - lastIndex) * segmentLength); // slope of the new section refers to the start of the section

            SuperelevationSection *section = new SuperelevationSection(sectionStart, superelevations[lastIndex], slope, 0.0, 0.0);
            newSections_.insert(sectionStart, section);

            if ((i == pointCount - 1) && (j != lastIndex))
            {
                lastIndex = i - 1;
                sectionStart = sStart + lastIndex * segmentLength;
                slope = (superelevations[i] - superelevations[lastIndex]) / segmentLength;

                SuperelevationSection *section = new SuperelevationSection(sectionStart, superelevations[lastIndex], slope, 0.0, 0.0);
                newSections_.insert(sectionStart, section);
            }
            else
            {
                lastIndex = i - 1;
                sectionStart = sStart + lastIndex * segmentLength;
                slope = (superelevations[i] - superelevations[lastIndex]) / segmentLength;
            }

        }
    }

#if 0
	double pointsPerMeter = 0.1; // BAD: hard coded!
	int pointCount = int(ceil((sEnd-sStart)*pointsPerMeter)); // TODO curvature...
	double segmentLength = (sEnd-sStart)/(pointCount-1);

	// Sections //
	//
	double superelevations[pointCount];
	SuperelevationSection * lastSection = NULL;
	for(int i = 0; i < pointCount; ++i)
	{
		double s = sStart + i * segmentLength; // [sStart, sEnd]

		// Height //
		//
		QPointF pos = road->getGlobalPoint(s);
		double height = 0.0;
		int count = 0;
		foreach(Heightmap * map, maps_)
		{
			if(map->isIntersectedBy(pos))
			{
				height = height + map->getHeightmapValue(pos.x(), pos.y());
				++count;
			}
		}
		if(count != 0)
		{
			height = height/count;
		}

		superelevations[i] = height + heightOffset_;
	}


	for(int i = 0; i < pointCount-1; ++i)
	{
		double s = sStart + i * segmentLength; // [sStart, sEnd]
		double slope = (superelevations[i+1]-superelevations[i])/segmentLength;
		if(lastSection
			&& (fabs(lastSection->getB() - slope) < NUMERICAL_ZERO6)
			&& (fabs(lastSection->getSuperelevation(s) - superelevations[i]) < NUMERICAL_ZERO6)
		)
		{
			continue;
		}

		SuperelevationSection * section = new SuperelevationSection(s, superelevations[i], slope, 0.0, 0.0);
		newSections_.insert(s, section);
		lastSection = section;
	}
#endif

    delete[] superelevations;
    // No sections created //
    //
    if (newSections_.isEmpty())
    {
        SuperelevationSection *section = new SuperelevationSection(0.0, 0.0, 0.0, 0.0, 0.0);
        newSections_.insert(0.0, section);
    }

    // Done //
    //
    setValid();
    setText(QObject::tr("Apply Heightmap"));
}

/*! \brief .
*
*/
ApplyHeightMapSuperelevationCommand::~ApplyHeightMapSuperelevationCommand()
{

    if (isUndone())
    {
        foreach (SuperelevationSection *section, newSections_)
        {
            delete section;
        }
        foreach (CrossfallSection *section, newCSections_)
        {
            delete section;
        }
    }
    else
    {
        foreach (SuperelevationSection *section, oldSections_)
        {
            delete section;
        }
        foreach (CrossfallSection *section, oldCSections_)
        {
            delete section;
        }
    }
}

/*! \brief .
*
*/
void
ApplyHeightMapSuperelevationCommand::redo()
{
    road_->setSuperelevationSections(newSections_);
    road_->setCrossfallSections(newCSections_);

    if (smoothLinear_)
    {
        SuperelevationSection *elevationSectionLow = road_->getSuperelevationSection(0.0);
        double highSEnd = elevationSectionLow->getSEnd();
        SuperelevationSection *elevationSectionHigh;
        double myDist = radius_;

        while (fabs(road_->getLength() - highSEnd) > NUMERICAL_ZERO3)
        {
            elevationSectionHigh = road_->getSuperelevationSection(highSEnd);
            elevationSectionLow = road_->getSuperelevationSectionBefore(highSEnd);

            if ((elevationSectionHigh->getLength() < elevationSectionLow->getLength()) && (elevationSectionHigh->getLength() < myDist))
            {
                myDist = elevationSectionHigh->getLength() / 2;
            }
            else if (elevationSectionLow->getLength() < myDist)
            {
                myDist = elevationSectionLow->getLength() / 2;
            }

            double slopeLow = (elevationSectionLow->getSuperelevationRadians(elevationSectionLow->getSEnd()) - elevationSectionLow->getSuperelevationRadians(elevationSectionLow->getSStart())) / elevationSectionLow->getLength();

            double slopeHigh = (elevationSectionHigh->getSuperelevationRadians(elevationSectionHigh->getSEnd()) - elevationSectionHigh->getSuperelevationRadians(elevationSectionHigh->getSStart())) / elevationSectionHigh->getLength();
            double angle = 180.0 - atan((slopeHigh - slopeLow) / (1 + slopeHigh * slopeLow));
            double tanAngle = tan(angle * M_PI / 360);
            double myRadius = fabs(myDist * tanAngle);

            highSEnd = elevationSectionHigh->getSEnd();
            SmoothSuperelevationSectionCommand *command = new SmoothSuperelevationSectionCommand(elevationSectionLow, elevationSectionHigh, myRadius);
            if (command->isValid())
            {
                command->redo();
            }
        }
    }

    setRedone();
}

/*! \brief
*
*/
void
ApplyHeightMapSuperelevationCommand::undo()
{
    road_->setSuperelevationSections(oldSections_);

    setUndone();
}

double
ApplyHeightMapSuperelevationCommand::getSuperelevation(double s)
{
    //QPointF posm = road_->getGlobalPoint(s);
    float wr = road_->getMaxWidth(s);
    float wl = road_->getMinWidth(s);
    QPointF posr = road_->getGlobalPoint(s, wr);
    QPointF posl = road_->getGlobalPoint(s, wl);
 /*   double heightm = 0.0;
    int countm = 0;*/
    double heightr = 0.0;
    int countr = 0;
    double heightl = 0.0;
    int countl = 0;

    // Take the average over all maps //
    //
    foreach (Heightmap *map, maps_)
    {
     /*   if (map->isIntersectedBy(posm))
        {
            heightm = heightm + map->getHeightmapValue(posm.x(), posm.y());
            ++countm;
        }*/
        if (map->isIntersectedBy(posr))
        {
            heightr = heightr + map->getHeightmapValue(posr.x(), posr.y());
            ++countr;
        }
        if (map->isIntersectedBy(posl))
        {
            heightl = heightl + map->getHeightmapValue(posl.x(), posl.y());
            ++countl;
        }
    }
  /*  if (countm != 0)
    {
        heightm = heightm / countm;
    }*/
    if (countr != 0)
    {
        heightr = heightr / countr;
    }
    if (countl != 0)
    {
        heightl = heightl / countl;
    }

    return atan((heightr - heightl) / (wr-wl))*180.0/M_PI;
}
