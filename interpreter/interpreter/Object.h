#pragma once
#include "common.h"
#include "Meta.h"

enum class OBJECT_TYPE{ INVAILD ,INTEGER,FLOAT};


struct Object {
	string name;
	OBJECT_TYPE type;
	union {
		Meta data;
		int value;
	};

	Object(string name,OBJECT_TYPE type):
	name(name),type(type){}
	Object() {}
};


class ObjectPool {
public:
	static Object query_object(string name);
	//for object under 4 bytes we need to update it in order to change it
	static void update_object(int val,string name);
	static void declare(string name,OBJECT_TYPE type,uint size = 0);
private:
	Meta allocate(uint size);
	

	map<string, Object> objs;
	vector<Meta> mems;
	static ObjectPool instance;
};