#pragma once
#include "common.h"
#include "Meta.h"
#include "Token.h"
#include <stack>

enum class OBJECT_TYPE{ INVAILD ,INTEGER,FLOAT,STRING,FUNC,REF};

struct ASTNode;
struct FuncSignature;
class Token;

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

struct RefObject : public Object {
	inline static RefObject& 
		cast_2_ref_object(Object& obj) {
		return *reinterpret_cast<RefObject*>(&obj);
	}

	void point_to(Object* obj) {
		data.data = obj;
	}

	Object& dereference() {
		return *static_cast<Object*>(data.data);
	}

	static Object create_object(MetaPool* pool,Object* ptr);
};

ostream& operator<<(ostream& out, Object obj);

class ObjectPool : public MetaPool{
public:
	static Object& query_object(string name);
	//for object under 4 bytes we need to update it in order to change it
	static void update_object(uint val,string name);
	static void update_object(Meta data,string name);
	//for some object may change it's size call this(for example string)
	static void update_dynamic_object(Meta data,string name);
	static void declare(string name,OBJECT_TYPE type,uint size = 0);

	static void define_func(FuncSignature& func);
	static ASTNode* invoke_func(string name,vector<Token>& param_list);
	static void end_invoke();
private:
	ObjectPool();

	stack<map<string, Object>> objs;
	stack<int> esp;

	map<string, Object> global;
	
	static ObjectPool instance;
};

inline uint pack_param_type(OBJECT_TYPE type,OBJECT_TYPE ref_type = OBJECT_TYPE::INVAILD) {
	return (uint)type | (uint)ref_type << 4;
}

inline OBJECT_TYPE get_param_type(uint packed_type ,bool is_ref = false) {
	if (is_ref) return (OBJECT_TYPE)((packed_type & 0xF0) >> 4);
	else return (OBJECT_TYPE)((packed_type)&0xF);
}


struct FuncSignature {
	ASTNode* func;
	string name;
	vector<string> ParamNames;
	vector<uint> ParamTypes;

	vector<string> DeclareName;
	vector<OBJECT_TYPE> DeclareType;
};


class FuncTable {
public:
	static bool regist_func(string name,FuncSignature& func);
	static bool decl_func(string name);
	static const FuncSignature* get_func_signature(string name);

private:
	static FuncTable func_table;

	map<string, FuncSignature> func_list;
};


bool Under4Bytes(OBJECT_TYPE type);

bool DynamicType(OBJECT_TYPE type);