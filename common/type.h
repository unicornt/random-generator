#ifndef __TYPE_H
#define __TYPE_H

#include "header.h"

class DependencyNode;

// 所有的基本类型都在这里
enum eSimpleType
{
	eVoid,
	eChar,
	eInt,
	eShort,
	eLong,
	eLongLong,
	eUChar,
	eUInt,
	eUShort,
	eULong,
	eFloat,
	eDouble,
	eULongLong,
	eInt128,
	eUInt128,
};

enum eTypeDesc
{
    eSimple,
    ePointer,
    eUnion,
    eStruct,
    eTypedef,
    eArray,
    eFunctionPointer,
};

/**
 * @brief 
 *    基类中包含 eType（类型），sid（序列号），name（类型名称），对所有子类生效
 *    ptr_type_sid（依赖类型的序列号） 为 0 ，ptr_type（指向依赖类型的指针） 为 nullptr 表示无效值，没有依赖
 * 1. 基本类型 eSimple：ptr_type 为空，eType 直接指向类型，name 为类型名称
 * 2. 复合类型 eUnion/eStruct ：ptr_type 为空，fields 指向成员的类型
 *    2.1 如果是普通复合类型，name 为 【struct/union + 类型名】格式
 *    2.2 如果是匿名复合类型，直接在变量上使用，通过isUname 判断，name 保留那一大长串位置信息
 *    2.3 如果是 typedef 的复合类型，通过 isTypedef 判断，name 为 typedef 的名称，不包含 struct/union
 * 3. 数组类型：ptr_type 指向元素类型，dimensions 表示维度，name 为 int[8] 格式（待进一步完善）
 * 4. 指针类型：ptr_type 指向元素类型，如果多重指针则只指向一次解引用的类型
 * 5. typedef 类型：ptr_type 指向真实的元素类型，name 表示新名称
 * 6. 函数指针类型，没怎么定义，几乎在当基础类型处理
 */
class TypeBase {
public:
    eTypeDesc eType;					// 类型描述
    unsigned int sid; 					// 类型的序列号，用于序列化
    string name;						// 类型全名在这里指定
    unsigned int ptr_type_sid;		    // 对于非基本类型使用，表示依赖类型的序列号
    TypeBase* ptr_type;                 // 对于非基本类型使用，恢复时赋值，指向依赖类型
    unsigned int qualifier;             // 修饰符，0x1 表示 const，0x2 表示 restrict，0x4 表示 volatile 
    TypeBase(eTypeDesc eType, unsigned int sid, string n):eType(eType), sid(sid), name(n), ptr_type_sid(0), ptr_type(nullptr) {}
    ~TypeBase() {}
    // 保存为 json
    virtual json dump();
    // 是否有依赖类型
    bool isBasicType();
    // 从json文件中恢复类型定义，只有 CompoundImpl 需要重载这个函数, 因为只有他依赖 field 来指向他的依赖类型，其余的通过 ptr_type 即可
    virtual void restorePtr(map<unsigned int, TypeBase*>& sid_type_map);
    // 获取这个类型的所有依赖类型，是纯虚函数，子类需要明确进行定义
    virtual DependencyNode* saveDependency(set<string>& used_dependency_node)=0;
    // 获取类型的定义
    virtual string getDecl();
    // 获取类型的名称
    virtual string getName(string v_name);
    // 对类型进行哈希
    virtual string getHashStmt(string name, set<string>& hashed);
    // 获取没有语法糖的函数类型，只有 typedef 需要重载，其余只需要返回名称即可
    virtual TypeBase*getDesugaredTypePtr() {return this;}
    // 获取底层类型，直到为 eSimple 或者 Compound   
    TypeBase* getCannonicalTypePtr() {
        TypeBase* cur = this;
        while(cur->ptr_type != nullptr) {
            cur = cur->ptr_type;
        } 
        return cur;
    }
    // 判断是否为 const
    bool isConst() {return qualifier & 0x1;}
    virtual string getInitialization();
};

// int void ...
class SimpleImpl : public TypeBase {
public:
    // 去掉修饰符后的类型
    string UnqualifiedType;
    SimpleImpl(unsigned int sid, string n, string UnqT): TypeBase(eSimple, sid, n), UnqualifiedType(UnqT) {}
    json dump();
    ~SimpleImpl() {cout << "free simple" << endl;}
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getHashStmt(string name, set<string>& hashed);
    string getInitialization();
};

// [simple/struct/pointer/typedef]*
class PointerImpl : public TypeBase {
public:
    // 需要额外赋值 ptr_type
    PointerImpl(unsigned int sid, string n): TypeBase(ePointer, sid, n) {};
    ~PointerImpl() {}
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getHashStmt(string name, set<string>& hashed);
    string getInitialization();
};

// struct/union
class CompoundImpl : public TypeBase {
public:
    vector<unsigned int> fields_type;       // 【复合类型】：struct/union 的 field 类型 sid
    vector<TypeBase*> fields_type_ptr;       // 【复合类型】：struct/union 的 field 类型
    vector<string> fields_name;       // 【复合类型】：struct/union 的 field 名称
    bool isUnname;						            // 【复合类型】：struct/union 是否为匿名函数
    bool isTypedef;                       // 【复合类型】：struct/union 是否为typedef
    // 需要额外赋值 field
    CompoundImpl(unsigned int sid, string n, bool isUnion): TypeBase(isUnion ? eUnion : eStruct, sid, n), isUnname(false), isTypedef(false) {};
    ~CompoundImpl() {}
    json dump();
    void restorePtr(map<unsigned int, TypeBase*>& sid_type_map);
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getDecl();
    string getHashStmt(string name, set<string>& hashed);
    string getName(string v_name);
    string getInitialization();
};

// typedef int unint_32
class TypedefImpl: public TypeBase {
public:
    TypedefImpl(unsigned int sid, string n): TypeBase(eTypedef, sid, n) {};
    ~TypedefImpl() {}
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getDecl();
    string getHashStmt(string name, set<string>& hashed);
    TypeBase*getDesugaredTypePtr();
    string getInitialization();
};

// 数组
class ArrayImpl : public TypeBase {
public:
    vector<unsigned int> dimensions;    // 【数组类型】：数组的维度（目前只有一维）
    ArrayImpl(unsigned int sid, string n, vector<unsigned int> dim):TypeBase(eArray, sid, n), dimensions(dim) {};
    ~ArrayImpl() {}
    json dump();
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getHashStmt(string name, set<string>& hashed);
    string getName(string v_name);
    string getInitialization();
};

// 函数指针
class FunctionPinterImpl : public TypeBase {
public:
    FunctionPinterImpl(unsigned int sid, string n): TypeBase(eFunctionPointer, sid, n) {};
    ~FunctionPinterImpl() {}
    DependencyNode* saveDependency(set<string>& used_dependency_node);
    string getName(string v_name);  
    string getInitialization();
};

TypeBase* restoreType(json j);

class DependencyNode {
public:
    TypeBase* type;
    vector<DependencyNode*> dependency;
    string print(set<string>& printed_dependency_node);
    DependencyNode(): type(nullptr), dependency(vector<DependencyNode*>()) {};
    string getInitialization();
};

#endif