#ifndef COMMON_H
#define COMMON_H

#include "header.h"
#include "type.h"

extern vector<int> randRecord;
extern int randAction;

// class Struct;

class Pos {
public:
  int line, column;
  Pos():line(0),column(0){};
  Pos(int l, int c): line(l), column(c) {}; 
  bool operator< (const Pos& b);
  bool operator== (const Pos& b);
  string str(); 
  json dump();
  static Pos restore(json j);
};


class Variable {
public:
  string type;
  unsigned int type_sid;
  TypeBase* type_ptr;
  string name;
  Pos declRef;
  // Struct* structPointer;      // 方便判断 var 是不是一个 struct 类型
  Variable(string t, string n):type(t), name(n), 
    // structPointer(nullptr), 
  declRef(Pos(0, 0)), type_sid(-1), type_ptr(nullptr) {};
  string str();
  bool operator==(const Variable& b);
  json dump();
  static Variable restore(json j, map<unsigned int, TypeBase*>& sid_type_map);
};

// // 记录类型别名
// class Typedef : public DependencyNode {
// public:
//   string type;
//   Struct* structPointer;
//   int isUnname;
//   Typedef(){};
//   Typedef(string n, string t): type(t), structPointer(nullptr), isUnname(0) {name = n;};
//   string str();
//   json dump();
//   static Typedef restore(json j);
// };

// 记录函数签名
class Signature {
public:
  Pos start;
  vector<Variable> params;
  Variable returnValue;
  string name;
  // 函数签名，用于判定两个函数是否相同，sig = "${returnValue.type} ${name}(${params.type params.name},{..})"" 
  string sig;
  // 返回值默认void
  Signature():returnValue(Variable("void", "r1")){};
  json dump();
  static Signature restore(json j, map<unsigned int, TypeBase*>& sid_type_map);
};


class Brick {
public:
  // 全局 id  file-id
  string id;
  // 语句的开始结束行列号
  Pos start, end;
  // 对于 for if 等复合语句的 body 起始处
  Pos bodyStart;
  // 语句需要的前置变量和后续变量
  vector<Variable> preContext, postContext;
  // 引用到的函数
  vector<Signature> preFunc;
  // 代码
  string code;
  // 语句的类型
  string type;
  // 语句所在文件名
  string file;
  
  // unknown, return, 
  Brick() {};
  Brick(Pos s, Pos e) {start=s;end=e;type="unknown";Pos bodyStart(0,0);};
  void print();
  string rename(map<int, string> rename_map);
  json dump();
  static Brick restore(json j, map<unsigned int, TypeBase*>& sid_type_map);
};


// class Struct : public DependencyNode{
// public:
//   vector<Variable> fields;
//   int isUnion;
//   Struct():isUnion(0){};
//   json dump();
//   static Struct restore(json j);
//   bool operator== (const Struct& s);
//   string str();
// };

int getRand(int n);

vector<string> split(string code, char sep);

string join(char c, vector<string> src);

vector<int> getNfromRangeM(int n, int m);

vector<string> getMap(vector<Variable> context, vector<Variable> preContext);

map<int, string> isSubset(vector<Variable> parent, vector<Variable> child);

// void writeFileJson(string path, Value root);
void writeFileJson(string path, json j);

vector<string> getFiles(string dir);

string path_join(int num, ...);

string removeQualOfType(string type);

string getFuncPointerStr(string type, string name);

vector<string> findSizeofType(string code);

class Array {
public:
  string name;
  int size;
  Array(string n, int s): name(n), size(s) {};
  bool isValid() {return name != "" && size != -1;}
  // string str() {return name + "[" + to_string(size) + "]";}
};

Array* getArrayInfo(string type);



#endif