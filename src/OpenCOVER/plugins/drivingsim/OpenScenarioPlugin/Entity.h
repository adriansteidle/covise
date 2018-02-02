#ifndef ENTITY_H
#define ENTITY_H

#include<string>
#include <TrafficSimulation/AgentVehicle.h>

class Entity {

 public:
   std::string name;
   std::string catalogReferenceName;
   std::string filepath;
	float speed;
    std::string roadId;
	int laneId;
	float inits;
	AgentVehicle *entityGeometry;
	osg::Vec3 entityPosition;
	osg::Vec3 directionVector;

    Entity(std::string entityName, std::string catalogReferenceName);
	~Entity();
	void setInitEntityPosition(osg::Vec3 init);
	void setInitEntityPosition(Road *r);
    void moveLongitudinal();
    std::string &getName();
	void setSpeed(float speed_temp);

	float &getSpeed();
    osg::Vec3 &getPosition();
    void setPosition(osg::Vec3 &newPosition);   
	void setDirection(osg::Vec3 &newDirection);

    // follow Trajectory attributes
    osg::Vec3 targetPosition;
    osg::Vec3 totaldirectionVector;
    osg::Vec3 newPosition;

    int visitedVertices;
    float totalDistance;
    float totaldirectionVectorLength;

    // follow Trajectories functions
    void setTargetPosition(osg::Vec3 init_targetPosition);
    void followTrajectory(int verticesCounter,bool &maneuverCondition, bool &maneuverFinished);
    void getTrajSpeed(float deltat);

};

#endif // ENTITY_H
