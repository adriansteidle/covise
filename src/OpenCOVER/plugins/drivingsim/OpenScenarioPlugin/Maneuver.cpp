#include "Maneuver.h"
#include <cover/coVRPluginSupport.h>
#include <iterator>
#include <math.h>
#include "OpenScenarioPlugin.h"
#include "Act.h"

using namespace std;

Maneuver::Maneuver():
	maneuverCondition(false),
	maneuverFinished(false),
	totalDistance(0),
	visitedVertices(0),
	trajectoryCatalogReference(""),
	startAfterManeuver(""),
	startConditionType("termination"),
	targetSpeed(0)
{
}
Maneuver::~Maneuver()
{
}

void Maneuver::finishedParsing()
{
	name = oscManeuver::name.getValue();
}

osg::Vec3 &Maneuver::followTrajectory(osg::Vec3 currentPos, vector<osg::Vec3> polylineVertices, float timer)

{
    int verticesCounter = polylineVertices.size();

    if(totalDistance == 0)
    {

        verticeStartPos = currentPos;
        targetPosition = polylineVertices[visitedVertices];

        // calculate speed
        totaldirectionVector = targetPosition - verticeStartPos;
        totaldirectionVectorLength = totaldirectionVector.length();
        speed = totaldirectionVectorLength/deltat;

        fprintf(stderr,"%f, ",timer);
        fprintf(stderr, "%s, %i, %f, %f,\n",name.c_str(),visitedVertices,currentPos[0],targetPosition[0]);

    }




    //substract vectors
    directionVector = targetPosition - currentPos;
    float distance = directionVector.length();
    directionVector.normalize();

    //calculate step distance
    float step_distance = speed*opencover::cover->frameDuration();
    //float step_distance = speed*1/60;

    if(totalDistance <= 0)
	{
		totalDistance = distance;
	}
	//calculate remaining distance
	totalDistance = totalDistance-step_distance;
	//calculate new position
	newPosition = currentPos+(directionVector*step_distance);
    if (totalDistance <= 0)
	{
		visitedVertices++;
		totalDistance = 0;
		if (visitedVertices == verticesCounter)
		{
			maneuverCondition = false;
			maneuverFinished = true;
		}
	}
	return newPosition;
}

osg::Vec3 &Maneuver::followTrajectoryRel(osg::Vec3 currentPos, osg::Vec3 targetPosition,float speed)
{

    //substract vectors
    directionVector = totaldirectionVector;//targetPosition - currentPos;
    float distance = directionVector.length();
    directionVector.normalize();
    //calculate step distance
    //float step_distance = speed*opencover::cover->frameDuration();
    float step_distance = speed*1/60;

    if(totalDistance == 0)
    {
        totalDistance = distance;
    }
    //calculate remaining distance
    totalDistance = totalDistance-step_distance;
    //calculate new position
    newPosition = currentPos+(directionVector*step_distance);
    if (totalDistance <= 0)
    {
        visitedVertices++;
        totalDistance = 0;
        if (visitedVertices == verticesCounter)
        {
            maneuverCondition = false;
            maneuverFinished = true;
        }
    }

    return newPosition;
}

void Maneuver::checkConditions()
{
    if (startConditionType == "time")
    {
        if (startTime<OpenScenarioPlugin::instance()->scenarioManager->simulationTime && maneuverFinished != true)
        {
            maneuverCondition = true;
        }
        else
        {
            maneuverCondition = false;
        }
    }
    if (startConditionType == "distance")
    {
        auto activeCar = OpenScenarioPlugin::instance()->scenarioManager->getEntityByName(activeCarName);
        auto passiveCar = OpenScenarioPlugin::instance()->scenarioManager->getEntityByName(passiveCarName);
        if (activeCar->entityPosition[0] - passiveCar->entityPosition[0] >= relativeDistance && maneuverFinished == false)
        {
            maneuverCondition = true;
        }

    }
    if (startConditionType == "termination")
    {
        for (list<Act*>::iterator act_iter = OpenScenarioPlugin::instance()->scenarioManager->actList.begin(); act_iter != OpenScenarioPlugin::instance()->scenarioManager->actList.end(); act_iter++)
        {
            for (list<Maneuver*>::iterator terminatedManeuver = (*act_iter)->maneuverList.begin(); terminatedManeuver != (*act_iter)->maneuverList.end(); terminatedManeuver++)
            {
                if ((*terminatedManeuver)->maneuverFinished == true && maneuverFinished == false && (*terminatedManeuver)->getName() == startAfterManeuver)
                {
                    maneuverCondition = true;
                }
            }
        }
    }
}

string &Maneuver::getName()
{
	return name;
}

void Maneuver::changeSpeedOfEntity(Entity *aktivCar, float dt)
{
	float negativeAcceleration = 50;
	float dv = negativeAcceleration*dt;
	if(aktivCar->getSpeed()>targetSpeed)
	{
		aktivCar->setSpeed(aktivCar->getSpeed()-dv);
	}
	else
	{
	aktivCar->setSpeed(targetSpeed);
	}
}
osg::Vec3 &Maneuver::setTargetPosition(osg::Vec3 currentPos,vector<osg::Vec3> polylineVertices,vector<bool> isRelVertice){
    verticesCounter = polylineVertices.size();


    if(totalDistance == 0){

        verticeStartPos = currentPos;
        if(isRelVertice[visitedVertices] == true){
            polylineVertices[visitedVertices] = currentPos + polylineVertices[visitedVertices];
        }
        targetPosition = polylineVertices[visitedVertices];
        totaldirectionVector = targetPosition - verticeStartPos;

    }
    //return targetPosition;
}


float &Maneuver::getTrajSpeed(osg::Vec3 verticeStartPos, osg::Vec3 targetPosition)
{
    if(totalDistance == 0)
    {
        // calculate speed
        totaldirectionVectorLength = totaldirectionVector.length();
        speed = totaldirectionVectorLength/deltat;

    }
    return speed;
}
