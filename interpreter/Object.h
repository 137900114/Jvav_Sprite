#pragma once
#include "common.h"
#include "Meta.h"

enum class OBJECT_TYPE{ INVAILD ,INTEGER,FLOAT,STRING};


struct Object {
	string name;
	OBJECT_TYPE type;
	union {
		Meta data;
		uint value;
	};

	Object(string name,OBJECT_TYPE type):
	name(name),type(type){}
	Object() {}
};

//for some object that size changes dynamicly,indexed object saves 8 bytes for them for their 
//index in Meta Pool.

//IndexedObject's Meta Memory structure:
//
// index(8 bytes) | pool_ptr (8 bytes) | data(...bytes)
//
struct IndexedObject : public Object{
	inline static IndexedObject& 
		cast_2_indexed_object(Object& obj) {
		return *reinterpret_cast<IndexedObject*>(&obj);
	}

	static Object create_object(uint size, OBJECT_TYPE type, MetaPool* pool);

	void update_size(uint size);

	uint* access_index() {
		return reinterpret_cast<uint*>(data.data);
	}

	MetaPool** access_pool() {
		byte* pos = reinterpret_cast<byte*>(data.data) + sizeof(uint);
		return reinterpret_cast<MetaPool**>(pos);
	}

	void* access_data() {
		return  sizeof(uint) + sizeof(MetaPool*) + reinterpret_cast<byte*>(data.data);
	}

	uint size() { return data.meta_size - sizeof(uint) - sizeof(MetaPool*); }
};

ostream& operator<<(ostream& out, Object obj);

class ObjectPool : public MetaPool{
public:
	static Object query_object(string name);
	//for object under 4 bytes we need to update it in order to change it
	static void update_object(uint val,string name);
	static void update_object(Meta data,string name);
	//for some object may change it's size call this(for example string)
	static void update_dynamic_object(Meta data,string name);
	static void declare(string name,OBJECT_TYPE type,uint size = 0);
private:
	map<string, Object> objs;
	
	static ObjectPool instance;
};

bool Under4Bytes(OBJECT_TYPE type);

bool DynamicType(OBJECT_TYPE type);