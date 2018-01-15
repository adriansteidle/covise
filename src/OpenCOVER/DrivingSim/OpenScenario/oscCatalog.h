/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */

#ifndef OSC_CATALOG_BASE_H
#define OSC_CATALOG_BASE_H

#include "oscExport.h"
#include "oscObjectBase.h"
#include "schema/oscFileHeader.h"

#include "schema/oscDirectory.h"
#include "schema/oscUserDataList.h"

#include <vector>
#if __cplusplus >= 201103L || defined WIN32
#include <unordered_map>
using std::unordered_map;
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include <string.h>


namespace bf = boost::filesystem;




namespace OpenScenario {

	class OPENSCENARIOEXPORT oscCatalogFile
	{
	public:
		oscCatalogFile(const std::string &catalogName, const std::string &filename, const std::string &path);
		~oscCatalogFile();
		const bf::path &getPath();
		void setPath(const bf::path &fn);
		oscFileHeader *m_Header;
		std::string catalogName;
		oscSourceFile *srcFile;
		void writeCatalogToDOM();
		std::vector<oscCatalogFile *>xoscFiles;
		std::vector<oscObjectBase *> objects;
		void removeObject(oscObjectBase *);
		void addObject(oscObjectBase *);
	private:
		bf::path fileName;
	};
	/// \class This class represents a catalog object
class OPENSCENARIOEXPORT oscCatalog: public oscObjectBase
{
	
public:
	typedef struct 
	{
		oscCatalogFile *xoscFile;
		oscObjectBase *object;
	} ObjectParams;

    oscCatalog()
    {
        OSC_OBJECT_ADD_MEMBER(Directory, "oscDirectory", 0);
    };

    oscDirectoryMember Directory;

	
    typedef unordered_map<std::string /*m_catalogType*/, std::string /*catalogTypeName*/> CatalogTypeTypeNameMap;
	typedef unordered_map<std::string, ObjectParams> ObjectsMap; ///< represent the unordered_map of objects
	
protected:
    std::string m_catalogName; ///< type of the objects in this catalog, e.g. vehicle, pedestrian
	std::string m_catalogType;
	ObjectsMap m_Objects;
	std::vector<oscCatalogFile *>xoscFiles;
	
public:
    //
	oscCatalogFile *getCatalogFile(int index);
	oscCatalogFile *getCatalogFile(const std::string &catalogName, const std::string &path);
	void clearAllCatalogs();
	void fastReadCatalogObjects(); ///< parse files and add objectRefId and filePath to ObjectsMap
    void getXoscFilesFromDirectory(); ///< find xosc file recursively in given directory

    //catalogType
    void setCatalogNameAndType(const std::string &catalogName);
    std::string getCatalogName() const;
	std::string getCatalogType() const;

    //availableObjects
    void setObjectsMap(const ObjectsMap &availableObjects);
	size_t getNumObjects() { return m_Objects.size(); };
    const ObjectsMap &getObjectsMap() const;
    bool addObjToObjectsMap(const std::string &name, oscCatalogFile *catf, oscObjectBase *object);
    bool removeObjFromObjectsMap(const std::string &name);
	std::string getPath(const std::string &name);
	std::string getObjectPath(OpenScenario::oscObjectBase *object);
	OpenScenario::oscObjectBase *getObjectfromPath(const std::string &path);

    //ObjectsInMemory
    bool fullReadCatalogObjectWithName(const std::string &name); ///< read file for given objectRefId, generate the object structure and add object to ObjectsMap map
    //bool fullReadCatalogObjectFromFile(const bf::path &fileNamePath); ///< read file, get objectRefId, check and add to ObjectsMap, generate the object structure and add object to ObjectsMap 
   // bool addCatalogObject(oscObjectBase *objectBase); ///< read objectRefId and fileNamePath from oscObjectBase and add entries to ObjectsMap
    bool addCatalogObject(const std::string &name, oscObjectBase *objectBase, oscCatalogFile *catFile); ///< add objectRefId and fileName and objectPtr to ObjectsMap
	void renameCatalogObject(oscObjectBase *,const std::string &name);
   // bool removeCatalogObject(const std::string &name); ///< remove object with refId objectRefId from ObjectsMap
	oscObjectBase *getCatalogObject(const std::string &name); ///< return pointer to oscObjectBase for objectRefId from ObjectsMap
	oscObjectBase *getCatalogObject(const std::string &catalogName, const std::string &entryName); ///< return pointer to oscObjectBase 


	//generate refId for new object
	std::string generateRefId();

	// write all catalog members to catalogs
	void writeCatalogsToDOM();
	void clearDOMs();
	void writeCatalogsToDisk();
	
    virtual bool parseFromXML(xercesc::DOMElement *currentElement, oscSourceFile *src, bool saveInclude = true);
    virtual bool writeToDOM(xercesc::DOMElement *currentElement, xercesc::DOMDocument *document, bool writeInclude = true);

private:
    typedef std::pair<bool, int> SuccessIntVar;

    SuccessIntVar getIntFromIntAttribute(xercesc::DOMAttr *attribute); ///< read an attribute of type oscMemberValue::INT and return int
};

typedef oscObjectVariable<oscCatalog *> oscCatalogMember;

}

#endif /* OSC_CATALOG_BASE_H */
