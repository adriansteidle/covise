#include "coMUIConfigParser.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <iostream>
#include <fstream>

using namespace std;
using namespace xercesc_3_1;


// constructor:
coMUIConfigParser::coMUIConfigParser(const std::string xmlAdresse){
    keywordVisible = "visible";
    keywordParent = "parent";
    keywordPosition = "position";
    keywordLabel = "label";
    keywordClass = "class";

    XMLPlatformUtils::Initialize();

    parser.reset(new XercesDOMParser());                         // create new xercesc-parser
    parser->setValidationScheme(xercesc::XercesDOMParser::Val_Never);

    initializeParser(xmlAdresse);
}

// destructor:
coMUIConfigParser::~coMUIConfigParser(){
}

// returns true, if the element shall be visible; else return false
bool coMUIConfigParser::getIsVisible(const std::string UI, const std::string Klasse, const std::string Instanz){
    if (existElement(UI, keywordClass, Klasse, nodeList)){                  // element with name UI and class Klasse exists
        UIElementNode = getElementNode(UI, keywordClass ,Klasse, nodeList); // UIElementNode is the elementnode with name UI and class Klasse
        if (existElement(Instanz, keywordVisible, "false", UIElementNode->getChildNodes())){
            return false;
        }
    }
    return true;
}

// returns the matching parentname
std::pair<std::string, bool> coMUIConfigParser::getParent(const std::string UI, const std::string Klasse, const std::string Instanz){
    std::pair<std::string, bool> returnPair;
    if (existElement(UI, keywordClass, Klasse, nodeList)){                  // element exists
        UIElementNode=getElementNode(UI, keywordClass, Klasse, nodeList);
        if (existElement(Instanz, keywordParent, UIElementNode->getChildNodes())){
            if (getAttributeValue(Instanz, keywordParent, UIElementNode->getChildNodes()).second){
                return getAttributeValue(Instanz, keywordParent, UIElementNode->getChildNodes());
            }
        }
    }
    returnPair.first="coMUIConfigParser::getParent(): Parent according to " + UI + " " + Klasse + " " + Instanz + " doesn't exist.";
    returnPair.second=false;
    return returnPair;
}

// returns the matching label
std::pair<std::string, bool> coMUIConfigParser::getLabel(const std::string UI, const std::string Klasse, const std::string Instanz){
    std::pair<std::string, bool> returnPair;
    existElement(UI, keywordClass, Klasse, nodeList);
    if (existElement(UI, keywordClass, Klasse, nodeList)){
        UIElementNode = getElementNode(UI, keywordClass, Klasse, nodeList);
        if (existElement(Instanz, keywordLabel, UIElementNode->getChildNodes())){
            if (getAttributeValue(Instanz, keywordLabel, UIElementNode->getChildNodes()).second){
                return getAttributeValue(Instanz, keywordLabel, UIElementNode->getChildNodes());
            }
        }
    }
    returnPair.first="coMUIConfigParser::getLabel(): Label according to " + UI + " " + Klasse + " " + Instanz + " doesn't exist.";
    returnPair.second=false;
    return returnPair;
}

// returns the matching position
std::pair<std::string, bool> coMUIConfigParser::getPosition(const std::string UI, const std::string Klasse, const std::string Instanz){
    std::pair<std::string, bool> returnPair;
    if (existElement(UI, keywordClass, Klasse, nodeList)){
        UIElementNode=getElementNode(UI, keywordClass, Klasse, nodeList);
        if (existElement(Instanz, keywordPosition, UIElementNode->getChildNodes())){
            return getAttributeValue(Instanz, keywordPosition, UIElementNode->getChildNodes());
        }
    }
    returnPair.first = "Position according to " + UI + " " + Klasse + " " + Instanz + " doesn't exist.";
    returnPair.second = false;
    return returnPair;
}

// returns true, if the file exists; else returns false
bool coMUIConfigParser::fileExist (std::string File){
    ifstream f(File.c_str());
    if (f.good()){
        f.close();
        return true;
    }
    else {
        f.close();
        return false;
    }
}

// returns the UI-Type of the element
const std::string coMUIConfigParser::getType(DOMElement* Element){
    return std::string(XMLString::transcode(Element->getTagName()));
}

// returns true, if node is elementnode
bool coMUIConfigParser::isNodeElement(DOMNode* Node){
    if (Node->getNodeType()==DOMNode::ELEMENT_NODE){
        return true;
    }
    else {
        return false;
    }
}

// returns the matching elementnode
DOMNode *coMUIConfigParser::getElementNode(const std::string TagName, const std::string Attribute, const std::string AttributeValue, DOMNodeList* NodeListe){
    for (size_t i=0; i<(NodeListe->getLength()); ++i){                                         // loop through all elements in NodeList
        if (NodeListe->item(i)->getNodeType()==DOMNode::ELEMENT_NODE){
            DOMElement* nodeElement=static_cast<DOMElement*>(NodeListe->item(i));
            if (XMLString::transcode(nodeElement->getTagName()) == TagName){   // match in TagName
                AttrVal= XMLString::transcode(nodeElement->getAttribute(XMLString::transcode(Attribute.c_str())));
                if (!AttrVal.empty()){                                                      // entry in Attribute
                    if (AttrVal == AttributeValue){                                         // match
                        return NodeListe->item(i);
                    }
                }
            }
        }
    }
    return NULL;
}

// returns true, if the element exists; else return false
bool coMUIConfigParser::existElement(const std::string TagName, const std::string Attribute, DOMNodeList* NodeListe){
    for (size_t i=0; i<(NodeListe->getLength()); ++i){                                         // loop through all elements in NodeList
        if (NodeListe->item(i)->getNodeType()==DOMNode::ELEMENT_NODE){
            DOMElement* nodeElement=static_cast<DOMElement*>(NodeListe->item(i)); // transform node to element
            if (XMLString::transcode(nodeElement->getTagName()) == TagName){   // match in TagName
                AttrVal = XMLString::transcode(nodeElement->getAttribute(XMLString::transcode(Attribute.c_str())));
                if (!AttrVal.empty()){                                                      // entry in Attribute
                    return true;
                }
            }
        }
    }
    return false;
}

// returns true, if the element exists; else return false
bool coMUIConfigParser::existElement(const std::string TagName, const std::string Attribute, const std::string AttributeValue, DOMNodeList* NodeListe){
    for (size_t i=0; i<(NodeListe->getLength()); ++i){                                         // loop through all elements in NodeList
        if (NodeListe->item(i)->getNodeType()==DOMNode::ELEMENT_NODE){
            DOMElement* nodeElement=static_cast<DOMElement*>(NodeListe->item(i)); // transform node to element
            if (XMLString::transcode(nodeElement->getTagName()) == TagName){   // match in TagName
                AttrVal = XMLString::transcode(nodeElement->getAttribute(XMLString::transcode(Attribute.c_str())));
                if (AttrVal == AttributeValue){                                             // entry in Attribute
                    return true;
                }
            }
        }
    }
    return false;
}

// retrurns the attributevalue as std::string
std::pair<std::string, bool> coMUIConfigParser::getAttributeValue(const std::string TagName, const std::string Attribute, DOMNodeList* NodeListe){
    std::pair<std::string, bool> returnPair;
    for (size_t i=0; i<(NodeListe->getLength()); ++i){                                         // loop through all elements in NodeList
        if (NodeListe->item(i)->getNodeType()==DOMNode::ELEMENT_NODE){
            boost::shared_ptr<DOMElement> nodeElement(static_cast<DOMElement*>(NodeListe->item(i))); // transform node to element
            if (XMLString::transcode(nodeElement->getTagName()) == TagName){   // match in TagName
                AttrVal = XMLString::transcode(nodeElement->getAttribute(XMLString::transcode(Attribute.c_str())));
                if (!AttrVal.empty()){                                                      // entry in Attribute
                    returnPair.first=AttrVal;
                    returnPair.second=true;
                    return returnPair;
                }
            }
        }
    }
    returnPair.first = "Attribute doesen't exist";
    returnPair.second = false;
    return returnPair;
}

// initializes parser with adress of configuration file
void coMUIConfigParser::initializeParser(std::string adress){
    if (fileExist(adress)){                                                                 // check if configuration file exists
        try{
            parser->parse(XMLString::transcode(adress.c_str()));               // parse configuration file
        }
        catch(...){
            cerr << "coMUIConfigParser.cpp: error parsing XML-Datei" << endl;
            cout << "coMUIConfigParser.cpp: Parser konnte XML-Datei nicht parsen" << endl;
        }

        parsedDoc=parser->getDocument();                                             // save configuration file as DOMDocument

        rootElement=parsedDoc->getDocumentElement();

        nodeList=rootElement->getChildNodes();
    }
    else {                                                                                  // File doesn't exist
        cerr << endl << "****************************************************************************************" << endl;
        cerr << "xml-Config-Datei konnte nicht gefunden werden. Bitte Namen und Pfad der Datei überprüfen" << endl;
        cerr << "****************************************************************************************" <<endl << endl;
    }
}

// lkets the parser read a new file
void coMUIConfigParser::readNewFile(std::string Filename){
    if (fileExist(Filename)){                                                               // check if configuration file exists
        try{
            parser->parse(XMLString::transcode(Filename.c_str()));             // parse configuration file
        }
        catch(...){
            cerr << "coMUIConfigParser.cpp: error parsing XML-Datei" << endl;
            cout << "coMUIConfigParser.cpp: Parser konnte XML-Datei nicht parsen" << endl;
        }

        parsedDoc=parser->getDocument();                                             // save configuration file as DOMDocument

        rootElement=parsedDoc->getDocumentElement();

        nodeList=rootElement->getChildNodes();
    }
    else {                                                                                  // File doesn't exist
        cerr << endl << "****************************************************************************************" << endl;
        cerr << "xml-Config-Datei konnte nicht gefunden werden. Bitte Namen und Pfad der Datei überprüfen" << endl;
        cerr << "****************************************************************************************" <<endl << endl;
    }
}
