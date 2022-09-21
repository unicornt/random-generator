#include "type.h"


string typeName[] = {
    "void",
    "char",
    "int",
    "short",
    "long",
    "longlong",
    "uchar",
    "uint",
    "ushort",
    "ulong",
    "float",
    "double",
    "ulonglong",
    "int128",
    "uint128"
};

/** ================================================================== /
 *                             Dump
 * ================================================================== */

json TypeBase::dump() {
  json j;
  j["eType"] = eType;
  j["sid"] = sid;
  j["name"] = name;
  j["ptr_type_sid"] = ptr_type_sid;
  j["qualifier"] = qualifier;
  return j;
}

json SimpleImpl::dump() {
  json j = TypeBase::dump();
  j["UnqType"] = UnqualifiedType;
  return j;
}

json CompoundImpl::dump() {
    json j = TypeBase::dump();
    json field_json_t;
    for(unsigned int f : fields_type) {
        field_json_t.push_back(f);
    }
    j["fields_type"] = field_json_t;
    json field_json_n;
    for(string f : fields_name) {
        field_json_n.push_back(f);
    }
    j["fields_name"] = field_json_n;
    j["isUnname"] = isUnname;
    j["isTypedef"] = isTypedef;
    return j;
}

json ArrayImpl::dump() {
    json j = TypeBase::dump();
    json dim_j;
    for(int d : dimensions) dim_j.push_back(d);
    j["dim"] = dim_j;
    return j;
}

/** ================================================================== /
 *                             Restore
 * ================================================================== */

void TypeBase::restorePtr(map<unsigned int, TypeBase*>& sid_type_map) {
    // 已经处理完成的不需要重复处理
  if(ptr_type != nullptr) return;
  if(!isBasicType()) ptr_type = sid_type_map[ptr_type_sid];
  return;
}

void CompoundImpl::restorePtr(map<unsigned int, TypeBase*>& sid_type_map) {
    // 已经处理完成的不需要重复处理
  if(fields_type_ptr.size() == fields_type.size()) return;
  for(unsigned int f : fields_type) {
    TypeBase* f_ptr = sid_type_map[f];
    f_ptr->restorePtr(sid_type_map);
    fields_type_ptr.push_back(f_ptr);
  }
  return;
}



/** ================================================================== /
 *                    Declaration && Name
 * ================================================================== */

string DependencyNode::print(set<string>& printed_dependency_node) {
  string res = "";
  for(DependencyNode* d : dependency) {
    res += d->print(printed_dependency_node);
    res += "\n";
  }
  // root.type == nullptr，同时为了保证已经输出的不会重复输出
  if(type != nullptr && printed_dependency_node.find(type->name) == printed_dependency_node.end()) {
    res += type->getDecl();
    res == "\n";
  }
  return res;
}

string TypeBase::getDecl() {
  // 一般来说，应该只有这两个能够调用
  assert("This type shouldn't getdecl by dependency !");
  return "";
}

string TypedefImpl::getDecl() {
  string res = "";
  switch (ptr_type->getDesugaredTypePtr()->eType){
    case eFunctionPointer: {
      res += "typedef ";
      res += ptr_type->getName(name);
      res += " ;\n";
      break;
    }
    case eStruct:
    case eUnion: {
      res += "typedef ";
      res += ptr_type->getName(name);
      res += " ";
      res += name;
      res += " ;\n";
      break;
    }
    default:
      // 其余类型不进行输出
      break;
  }
  return res;
}

string CompoundImpl::getDecl() {
  // TODO：unname
  string res = "";
  if(isUnname) res += (eType == eUnion) ? "union " : "struct ";
  else if(isTypedef) {
    res += "typedef ";
    res += (eType == eUnion) ? "union " : "struct ";
  } else res += name;
  res += " {\n";
  for(int i = 0; i < fields_type_ptr.size(); i++) {
    string tmp = "\t";
    switch (fields_type_ptr[i]->getDesugaredTypePtr()->eType){
      case eArray:
      case eFunctionPointer: {
        tmp += fields_type_ptr[i]->getDesugaredTypePtr()->getName(fields_name[i]);
        tmp += " ;\n";
        break;
      }
      default: {
        tmp += fields_type_ptr[i]->getName(fields_name[i]);
        tmp += " ";
        tmp += fields_name[i];
        tmp += ";\n";
        break;
      }
    }
    res += tmp;
  }
  res += "} ";
  if(isTypedef) res += name;
  res += " ;\n";
  return res;
}

// 如果不定义虚函数，默认返回名字
string TypeBase::getName(string v_name) {
  return name;
}

// 比较丑陋，先用字符串匹配了
string FunctionPinterImpl::getName(string v_name) {
  int index = name.find("(*)");
  string tmp = "";
  string partA = name.substr(0, index+2);
  string partB = name.substr(index+2, name.size() - partA.size());
  tmp += partA;
  tmp += v_name;
  tmp += partB;
  return tmp;
}

string ArrayImpl::getName(string v_name) {
  // TODO：多维数组
  int index = name.find("[");
  string tmp = "";
  string partA = name.substr(0, index);
  string partB = name.substr(index, name.size() - partA.size());
  tmp += partA;
  tmp += " ";
  tmp += v_name;
  tmp += partB;
  return tmp;
}

// 主要为了处理 Unname
string CompoundImpl::getName(string v_name) {
  if(isUnname) {
    string decl = getDecl();
    decl = decl.substr(0, decl.size()-2); // 去掉末尾的 ;\n
    return decl;
  } else {
    return name;
  }
}



/** ================================================================== /
 *                               Hash
 * ================================================================== */

string TypeBase::getHashStmt(string name, set<string>& hashed) {return "";}

string CompoundImpl::getHashStmt(string name, set<string>& hashed) {
  string res = "";
  res += "// hash compound variable " + name + " \n";
  for(int i=0;i<fields_type_ptr.size();i++) {
    string field_name = name + "." + fields_name[i];
    res += fields_type_ptr[i]->getHashStmt(field_name, hashed);
  }
  return res;
}

string TypedefImpl::getHashStmt(string name, set<string>& hashed) { return ptr_type->getHashStmt(name, hashed);}

// 先不处理指针了，这个有死循环的问题，比如链表
string PointerImpl::getHashStmt(string name, set<string>& hashed) { 
  // return ptr_type->getHashStmt("(*" + name + ")");
  return "";
}

string ArrayImpl::getHashStmt(string name, set<string>& hashed) { 
  return "";
}


string SimpleImpl::getHashStmt(string name, set<string>& hashed) {
  if(hashed.find(name) != hashed.end()) return "";
  hashed.insert(name);
  string res = "";
  res += "transparent_crc(";
  res += name;
  res += " , ";
  res += "\"" + name + "\"";
  res += " , ";
  res += "1);\n";
  return res;
}


/** ================================================================== /
 *                       Dependency Node
 * ================================================================== */


DependencyNode* TypeBase::saveDependency(set<string>& used_dependency_node) {
    return ptr_type->saveDependency(used_dependency_node);
}

// 不产生 dependency
DependencyNode* SimpleImpl::saveDependency(set<string>& used_dependency_node) {return nullptr;}
DependencyNode* FunctionPinterImpl::saveDependency(set<string>& used_dependency_node) {return nullptr;}

// 本身没有dependency，但是指向的类型可能有
DependencyNode* ArrayImpl::saveDependency(set<string>& used_dependency_node) {return ptr_type->saveDependency(used_dependency_node);}
DependencyNode* PointerImpl::saveDependency(set<string>& used_dependency_node) {return ptr_type->saveDependency(used_dependency_node);}

DependencyNode* CompoundImpl::saveDependency(set<string>& used_dependency_node) {
    if(used_dependency_node.find(name) != used_dependency_node.end()) return nullptr;
    used_dependency_node.insert(name);
    DependencyNode* d = new DependencyNode();
    d->type = this;
    for(TypeBase* f : fields_type_ptr) {
        DependencyNode* field_denpendency = f->saveDependency(used_dependency_node);
        if(field_denpendency) d->dependency.push_back(field_denpendency);
    }
    return d;
}

DependencyNode* TypedefImpl::saveDependency(set<string>& used_dependency_node) {
    if(used_dependency_node.find(name) != used_dependency_node.end()) return nullptr;
    used_dependency_node.insert(name);
    DependencyNode* d = new DependencyNode();
    d->type = this;
    DependencyNode* ptr_type_dependency = ptr_type->saveDependency(used_dependency_node);
    if(ptr_type_dependency) d->dependency.push_back(ptr_type_dependency);
    return d;
}

/** ================================================================== /
 *                               getInitialization
 * ================================================================== */

string TypeBase::getInitialization() {}
string SimpleImpl::getInitialization() {
  if(this->UnqualifiedType == typeName[1]){
    //char
    return "\'\'";
  }
  else {
    //int short ...
    return "0";
  }
  return "????????????";
}
string PointerImpl::getInitialization() {
  return "NULL";
}
string CompoundImpl::getInitialization() {
  string ret = "{";
  for(int i = 0; i < this->fields_name.size(); i++) {
    ret +=  this->fields_type_ptr[i]->getInitialization() + (i == this->fields_name.size() - 1 ? "}" : ", ");
  }
  return ret;
}
string TypedefImpl::getInitialization() {
  TypeBase* p = this;
  while(p->ptr_type != NULL) {
    p = p -> ptr_type;
  }
  return p->getInitialization();
}
string ArrayImpl::getInitialization() {}
string FunctionPinterImpl::getInitialization() {}
string DependencyNode::getInitialization() {}

/** ================================================================== /
 *                               Others
 * ================================================================== */

TypeBase* TypedefImpl::getDesugaredTypePtr() {
  // TODO: int8_t * 解引用为 char *
  TypeBase* res = ptr_type;
  while(res->eType == eTypedef) {
    res = res->ptr_type;
  }
  return res;
}


bool TypeBase::isBasicType() {
  return !(eType == eArray || eType == ePointer || eType == eTypedef);
}

TypeBase* restoreType(json j) {
  unsigned int typeDesc;
  string name;
  unsigned int sid;
  unsigned int ptr_type_sid;
  j["eType"].get_to(typeDesc);
  j["name"].get_to(name);
  j["sid"].get_to(sid);
  j["ptr_type_sid"].get_to(ptr_type_sid);
  TypeBase* tp = nullptr;
  switch (typeDesc) {
    case eSimple: {
      string UnqualifiedType;
      j["UnqType"].get_to(UnqualifiedType);
      SimpleImpl* st = new SimpleImpl(sid, name, UnqualifiedType);
      tp = st;
      break;
    }
    case ePointer: {
      PointerImpl* pt = new PointerImpl(sid, name);
      pt->ptr_type_sid = ptr_type_sid;
      tp = pt;
      break;
    }
    case eUnion:      
    case eStruct: {
      CompoundImpl* ct = new CompoundImpl(sid, name, typeDesc == eUnion);
      j["isUnname"].get_to(ct->isUnname);
      j["isTypedef"].get_to(ct->isTypedef);
      json fields_type = j["fields_type"];
      for (json::iterator it = fields_type.begin(); it != fields_type.end(); ++it) {
        unsigned int f_sid;
        it->get_to(f_sid);
        ct->fields_type.push_back(f_sid);
      }
      json fields_name = j["fields_name"];
      for (json::iterator it = fields_name.begin(); it != fields_name.end(); ++it) {
        string name;
        it->get_to(name);
        ct->fields_name.push_back(name);
      }
      tp = ct;
      break;
    }
    case eTypedef: {
      TypedefImpl* tdt = new TypedefImpl(sid, name);
      tdt->ptr_type_sid = ptr_type_sid;
      tp = tdt;
      break;
    }
    case eArray: {
      json dims_json = j["dim"];
      vector<unsigned int> dims;
      for (json::iterator it = dims_json.begin(); it != dims_json.end(); ++it) {
        unsigned int dim;
        it->get_to(dim);
        dims.push_back(dim);
      }
      ArrayImpl* at = new ArrayImpl(sid, name, dims);
      at->ptr_type_sid = ptr_type_sid;
      tp = at;
      break;
    }
    case eFunctionPointer: {
      FunctionPinterImpl* fpt = new FunctionPinterImpl(sid, name);
      tp = fpt;
      break;
    }
    default:
      break;
  }
  j["qualifier"].get_to(tp->qualifier);
  return tp;
}

