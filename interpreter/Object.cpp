#include "Object.h"
#include "pack_data.h"

bool Under4Bytes(OBJECT_TYPE type) {
	switch (type)
	{
	case OBJECT_TYPE::INTEGER:
	case OBJECT_TYPE::FLOAT:
		return true;
	default:
		return false;
	}
}

bool DynamicType(OBJECT_TYPE type) {
	switch(type) {
	case OBJECT_TYPE::STRING:
		return true;
	}
	return false;
}

Object IndexedObject::create_object(uint size, OBJECT_TYPE type, MetaPool* pool) {
	uint index;
	size = size + sizeof(uint) + sizeof(MetaPool*);
	Meta mem = pool->allocate(size, &index);
	Object obj;
	obj.data = mem;
	obj.type = type;
	IndexedObject& indexed = cast_2_indexed_object(obj);
	*indexed.access_index() = index;
	*indexed.access_pool() = pool;

	return obj;
}

void IndexedObject::update_size(uint size) {
	uint index = *access_index();
	MetaPool* pool = *access_pool();
	size = sizeof(uint) + sizeof(MetaPool*) + size;
	Meta new_mem = pool->update_pool(index, size);
	data = new_mem;
	*access_index() = index;
	*access_pool() = pool;
}

void ObjectPool::declare(string name,OBJECT_TYPE type,uint size) {
	map<string, Object>& obj_map = instance.objs;
	if (obj_map.find(name) != obj_map.end())
		throw string("invaild declaration ! the object with the same name : ") + name;
	
	Object new_obj(name, type);
	if (Under4Bytes(type)) {
		obj_map[name] = new_obj;
	}
	else {
		if (DynamicType(type)) {
			Object obj = IndexedObject::create_object(size, type, &instance);
			obj.name = name;
			obj_map[name] = obj;
			return;
		}
		else {
			new_obj.data = instance.allocate(size);
			obj_map[name] = new_obj;
			return;
		}

		throw string("invaild object type");
	}
}

Object ObjectPool::query_object(string name) {
	 map<string,Object>& obj_map = instance.objs;
	 auto query = obj_map.find(name);
	 if (query == obj_map.end())
		 throw string("invaild object name : ") + name;

	 return query->second;
}

void ObjectPool::update_object(uint val,string name) {
	auto& obj_map = instance.objs;
	auto query = obj_map.end();
	if ((query = obj_map.find(name)) == obj_map.end()) {
		throw string("invaild object name : ") + name;
	}
	if (!Under4Bytes(query->second.type)) {
		throw string("you shouldn't update object's value by passing value to update_object function"
		" becuase the object's size is more than 4 bytes");
	}

	query->second.value = val;
}

void ObjectPool::update_object(Meta data,string name) {
	auto& obj_map = instance.objs;
	auto query = obj_map.find(name);
	if (query == obj_map.end())
		throw string("invaild object name : ");
	if (Under4Bytes(query->second.type))
		throw string("you shouldn't update object's value by passing meta data to update_object function"
			" becuase the object's size is less than 4 bytes");
	
	if (query->second.data.meta_size != data.meta_size)
		throw string("the object you update should have a fixed size");
	memcpy(query->second.data.data, data.data, query->second.data.meta_size);
}


void ObjectPool::update_dynamic_object(Meta data,string name) {
	Object obj = query_object(name);
	if (!DynamicType(obj.type)) {
		throw string("only dynamic type objects can update by calling update_dynamic_object : updaing " + name);
	}
	IndexedObject& Iobj = IndexedObject::cast_2_indexed_object(obj);
	if (Iobj.size() != data.meta_size) {
		Iobj.update_size(data.meta_size);
	}

	memcpy(Iobj.access_data(),data.data,data.meta_size);
	//CAUTION !!!the address of the object has changed we need to write it back to map!!!
	instance.objs[name] = Iobj;
}


ObjectPool ObjectPool::instance;

#define OBJECT_OUT_FORMAT(symbol,name,type,val) \
	printf("object:{ name : ( %s ) , type : ( %s ) , value : ( "##symbol##" ) }\n",\
		name,type,val); 

ostream& operator<<(ostream& out, Object obj) {
	IndexedObject& Iobj = IndexedObject::cast_2_indexed_object(obj);
	switch (obj.type)
	{
	case OBJECT_TYPE::INTEGER:
		OBJECT_OUT_FORMAT("%d",obj.name.c_str(),"int",obj.value);
		break;
	case OBJECT_TYPE::FLOAT:
		OBJECT_OUT_FORMAT("%f",obj.name.c_str(),"float",unpack_float(obj.value));
		break;
	case OBJECT_TYPE::STRING:
		OBJECT_OUT_FORMAT("'%s'",obj.name.c_str(),"string",(char*)Iobj.access_data());
		break;
	default:
		out << "(unkown)" << endl;
		break;
	}
	return out;
}