#include "Object.h"
#include "pack_data.h"
#include "system_invoke.h"

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
	map<string, Object>& obj_map = instance.objs.top();
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

Object& ObjectPool::query_object(string name) {
	 map<string,Object>& obj_map = instance.objs.top();
	 auto query = obj_map.find(name);
	 if (query == obj_map.end()) {
		 if((query = instance.global.find(name)) == instance.global.end())
			 throw string("invaild object name : ") + name;
	 }
	 if (query->second.type == OBJECT_TYPE::REF) {
		 RefObject& rObj = RefObject::cast_2_ref_object(query->second);
		 return rObj.dereference();
	 }
	 return query->second;
}

void ObjectPool::update_object(uint val,string name) {
	auto& obj_map = instance.objs.top();
	auto query = obj_map.end();
	if ((query = obj_map.find(name)) == obj_map.end()) {
		throw string("invaild object name : ") + name;
	}
	Object* target;
	if (query->second.type == OBJECT_TYPE::REF) {
		target = &RefObject::cast_2_ref_object(query->second).dereference();
	}
	else
		target = &query->second;
	if (!Under4Bytes(target->type)) {
		throw string("you shouldn't update object's value by passing value to update_object function"
		" becuase the object's size is more than 4 bytes");
	}

	target->value = val;
}

void ObjectPool::update_object(Meta data,string name) {
	auto& obj_map = instance.objs.top();
	auto query = obj_map.find(name);
	if (query == obj_map.end())
		throw string("invaild object name : ");
	
	Object* target;
	if (query->second.type == OBJECT_TYPE::REF) {
		target = &RefObject::cast_2_ref_object(query->second).dereference();
	}
	else
		target = &query->second;
	if (Under4Bytes(target->type))
		throw string("you shouldn't update object's value by passing meta data to update_object function"
			" becuase the object's size is less than 4 bytes");
	
	if (target->data.meta_size != data.meta_size)
		throw string("the object you update should have a fixed size");
	memcpy(target->data.data, data.data, target->data.meta_size);
}


void ObjectPool::update_dynamic_object(Meta data,string name) {
	auto query = instance.objs.top().find(name);
	if (query == instance.objs.top().end()) {
		throw string("invaild object name ") + name;
	}
	Object* obj = &query->second;
	bool ref_flag = false;
	if (obj->type == OBJECT_TYPE::REF) {
		obj = &RefObject::cast_2_ref_object(*obj).dereference();
		ref_flag = true;
	}
	
	if (!DynamicType(obj->type)) {
		throw string("only dynamic type objects can update by calling update_dynamic_object : updaing " + name);
	}
	IndexedObject& Iobj = IndexedObject::cast_2_indexed_object(*obj);
	if (Iobj.size() != data.meta_size) {
		Iobj.update_size(data.meta_size);
	}

	memcpy(Iobj.access_data(),data.data,data.meta_size);
	//CAUTION !!!the address of the object has changed we need to write it back to map!!!
	//if the object is a reference and we update it 
	//if(!ref_flag) instance.objs.top()[name] = Iobj;
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

ObjectPool::ObjectPool() {
	objs.push(map<string, Object>());
}

Object RefObject::create_object(MetaPool* pool,Object* ptr) {
	Object obj;
	obj.type = OBJECT_TYPE::REF;
	obj.data = pool->allocate(sizeof(Object*));

	RefObject::cast_2_ref_object(obj).point_to(ptr);

	return obj;
}

map<string, FuncSignature> func_list;

bool FuncTable::decl_func(string name) {
	if (func_table.func_list.find(name) != func_table.func_list.end()) {
		return false;
	}
	func_table.func_list[name] = FuncSignature();
	return true;
}

bool FuncTable::regist_func(string name,FuncSignature& sig) {
	auto& table = func_table.func_list;
	auto iter = table.find(name);
	if (iter == table.end())
		return false;
	iter->second = sig;
	return true;
}

const FuncSignature* FuncTable::get_func_signature(string name) {
	auto& table = func_table.func_list;
	auto iter = table.find(name);
	if (iter == table.end())
		return nullptr;
	return &iter->second;
}

void ObjectPool::define_func( FuncSignature& sig) {
	auto& table = instance.global;
	string name = sig.name;
	auto query = table.find(name);
	if (query != table.end() || !FuncTable::decl_func(name) || !FuncTable::regist_func(name,sig)) {
		throw string("invaild function definition : should not has a object with the same name");
	}
	Object obj;
	obj.name = name;
	obj.type = OBJECT_TYPE::FUNC;
	table[name] = obj;
}

ASTNode* ObjectPool::invoke_func(string name,vector<Token>& params) {
	if (sys_par_invoke sys_inv = check_sys_par_invoke(name)) {
		uint curr_esp = instance.get_curr_pool_size();
		sys_inv(params,&instance);
		instance.release_pool(curr_esp);
		return nullptr;
	}
	instance.esp.push(instance.get_curr_pool_size());
	
	const FuncSignature* sig = FuncTable::get_func_signature(name);
	if (!sig)  throw string("invaild function name ") + name;
	if (sig->ParamNames.size() != params.size())
		throw string("invaild function invoke parameter don't match arguments");
	map<string, Object> objs;
	map<string, Object>& current_table = instance.objs.top();
	for (int i = 0; i != params.size(); i++) {
		if (get_param_type(sig->ParamTypes[i]) == OBJECT_TYPE::REF) {
			if (params[i].type() != OBJECT)
				throw string("invaild function invoke : invaild parameter : a reference must refer to a object");
			OBJECT_TYPE ref_obj_type = get_param_type(sig->ParamTypes[i],true);
			string obj_name = unpack_string(params[i].data());
			auto query = current_table.find(obj_name);
			if (query == current_table.end())
				throw string("invaild function invoke : reference to a invaild object ") + obj_name;
			Object* obj_ref = &query->second;
			if (obj_ref->type != ref_obj_type)
				throw string("invaild function invoke : reference should has the same type with the object ");
			objs[sig->ParamNames[i]] = RefObject::create_object(&instance,obj_ref);
		}
		else {
			Token current_t;
			string str_buf;
			Object new_obj;
			OBJECT_TYPE curr_para_type = get_param_type(sig->ParamTypes[i]);
			new_obj.type = OBJECT_TYPE::INVAILD;
			if (params[i].type() == OBJECT) {
				Object obj = query_object(unpack_string(params[i].data()));
				
				switch (obj.type) {
				case OBJECT_TYPE::INTEGER:
					current_t.type() = INTEGER;
					current_t.value() = obj.value;
					break;
				case OBJECT_TYPE::FLOAT:
					current_t.type() = FLOAT;
					current_t.value() = obj.value;
					break;
				case OBJECT_TYPE::STRING:
					current_t.type() = STRING;
					str_buf = unpack_string(obj.data);
					current_t.data() = pack_string(str_buf,&instance);
					break;
				default:
					throw string("invaild function parameter type ");
				}
			}
			else {
				current_t = params[i];
			}

			switch (current_t.type()) {
			case INTEGER:
				if (curr_para_type == OBJECT_TYPE::INTEGER) {
					new_obj.type = OBJECT_TYPE::INTEGER;
					new_obj.value = current_t.value();
					new_obj.name = sig->ParamNames[i];
				}
				break;
			case FLOAT:
				if (curr_para_type == OBJECT_TYPE::FLOAT) {
					new_obj.type = OBJECT_TYPE::FLOAT;
					new_obj.name = sig->ParamNames[i];
					new_obj.value = current_t.value();
				}
				break;
			}
			if (current_t.type() == STRING && curr_para_type == OBJECT_TYPE::STRING) {
				new_obj = IndexedObject::create_object(current_t.data().meta_size,OBJECT_TYPE::STRING,&instance);
				IndexedObject& Iobj = IndexedObject::cast_2_indexed_object(new_obj);

				memcpy(Iobj.access_data(), current_t.data().data, current_t.data().meta_size);
			}
			if (new_obj.type == OBJECT_TYPE::INVAILD)
				throw string("invaild function call invaild parameter type");
			objs[sig->ParamNames[i]] = new_obj;
		}

	}

	instance.objs.push(objs);
	//deal with the declarations in functions
	for (int i = 0; i != sig->DeclareName.size(); i++)
		ObjectPool::declare(sig->DeclareName[i],sig->DeclareType[i]);

	return sig->func;
}

FuncTable FuncTable::func_table;

void ObjectPool::end_invoke() {
	instance.release_pool(instance.esp.top());
	instance.esp.pop();
	instance.objs.pop();
}