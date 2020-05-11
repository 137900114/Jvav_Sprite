#include "Object.h"

void ObjectPool::declare(string name,OBJECT_TYPE type,uint size) {
	map<string, Object>& obj_map = instance.objs;
	if (obj_map.find(name) != obj_map.end())
		throw string("invaild declaration ! the object with the same name : ") + name;
	
	Object new_obj(name,type);
	switch (type)
	{
	case OBJECT_TYPE::INTEGER:
		obj_map[name] = new_obj;
		break;
	default:
		throw string("invaild object type");
	}
}

Meta ObjectPool::allocate(uint size) {
	Meta mem = MetaAllocator::allocate(size);
	mems.push_back(mem);
	return mem;
}

Object ObjectPool::query_object(string name) {
	 map<string,Object>& obj_map = instance.objs;
	 auto query = obj_map.find(name);
	 if (query == obj_map.end())
		 throw string("invaild object name : ") + name;

	 return query->second;
}

void ObjectPool::update_object(int val,string name) {
	auto& obj_map = instance.objs;
	if (obj_map.find(name) == obj_map.end()) {
		throw string("invaild object name : ") + name;
	}
	if (obj_map[name].type != OBJECT_TYPE::INTEGER) {
		throw string("you shouldn't update object's value by update_object function"
			"you can just directly use the query_object function and get the pointer to object");
	}

	obj_map[name].value = val;
}



ObjectPool ObjectPool::instance;