#ifndef ODRID_HPP
#define ODRID_HPP
#include <stdint.h>
#include <qstring.h>

class odrID
{
public:
	enum IDType
	{
		ID_None = 0,
		ID_Road,
		ID_Junction,
		ID_Controller,
		ID_Fiddleyard,
		ID_PedFiddleyard,
		ID_Object,
		ID_Tile,
		ID_Bridge,
		ID_Vehicle,
		ID_Signal,
		NUM_IDs
	};
	odrID()
	{
		ID = -1;
		tileID = -1;
		type = ID_None;
		name = "uninitialized";
	};
	odrID(IDType t)
	{
		ID = -1;
		tileID = -1;
		type = ID_None;
		name = "uninitialized";
		type = t;
	};
	odrID(int32_t _id, int32_t _tileID, const QString &_name, IDType t)
	{
		ID = _id;
		tileID = _tileID;
		name = _name;
		type = t;
	};
	odrID(const odrID &o) // creates an copy of an ID
	{
		ID = o.ID;
		tileID = o.tileID;
		name = o.name;
		type = o.type;
	};
	QString speakingName() const {
		QString s = QString::number(ID) + "_" + QString::number(tileID) + "_" + name;
		if (type == ID_Road)
			return "r_" + s;
		if (type == ID_Junction)
		    return "j_" + s;
		if (type == ID_Controller)
			return "c_" + s;
		if (type == ID_Fiddleyard)
			return "f_" + s;
		if (type == ID_PedFiddleyard)
			return "pf_" + s;
		if (type == ID_Object)
			return "o_" + s;
		if (type == ID_Bridge)
			return "b_" + s;
		if (type == ID_None)
			return "none_" + s;
		if (type == ID_Signal)
			return "s_" + s;
		if (type == ID_Tile)
			return "t_" + s;
		if (type == ID_Vehicle)
			return "v_" + s;
		return "unknownType_" + s;
	};
	QString writeString() const {// different write options
		QString s = QString::number(ID) + "_" + QString::number(tileID) + "_" + name;
		if (type == ID_Road)
			return "r_" + s;
		if (type == ID_Junction)
			return "j_" + s;
		if (type == ID_Controller)
			return "c_" + s;
		if (type == ID_Fiddleyard)
			return "f_" + s;
		if (type == ID_PedFiddleyard)
			return "pf_" + s;
		if (type == ID_Object)
			return "o_" + s;
		if (type == ID_Bridge)
			return "b_" + s;
		if (type == ID_None)
			return "none_" + s;
		if (type == ID_Signal)
			return "s_" + s;
		if (type == ID_Tile)
			return "t_" + s;
		if (type == ID_Vehicle)
			return "v_" + s;
		return "unknownType_" + s;
	};
	int32_t getID() const { return ID; };
	int32_t getTileID() const { return tileID; };
	IDType getType() const { return type; };
	const QString & getName() const { return name; };
	void setName(const QString &n) { name = n; };
	void setTileID(uint32_t ti) { tileID = ti; };
	void setID(uint32_t i) { ID = i; };
	void setTileID(const odrID &i) { tileID = i.getID(); };
	void setType(IDType t) { type = t; };
	bool operator !=(const odrID &i) const { return ID != i.ID || type != i.type; };
	bool operator ==(const odrID &i) const { return ID == i.ID && type == i.type; };
	bool operator <(const odrID &i) const { return ID < i.ID; };
	bool operator >(const odrID &i) const { return ID > i.ID; };
	static const odrID& invalidID() { return noID; };
	bool isValid() const { return ID != -1; };
	bool isInvalid() const { return ID == -1; };
private:

	int32_t ID;
	int32_t tileID;
	QString name;
	static odrID noID;
	IDType type;
};

#endif //ODRID_HPP
