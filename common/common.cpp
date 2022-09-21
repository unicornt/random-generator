#include "common.h"

bool Pos::operator< (const Pos& b){
  if(line < b.line) {
    return true;
  } else {
    if(line == b.line) {
      return column <= b.column;
    } else {
      return false;
    }
  }
}
bool Pos::operator== (const Pos& b) {
  return line == b.line && column == b.column;
}

string Pos::str() {
  return to_string(line) + ":" + to_string(column);
}

json Pos::dump() {
  json j;
  j["line"] = line;
  j["column"] = column;
  return j;
}

Pos Pos::restore(json j) {
  int line, column;
  j["line"].get_to(line);
  j["column"].get_to(column);
  Pos p = Pos(line, column);
  // cout << p.str() << endl;
  return p;
}



string Variable::str() {
  return "name:\t"+name+"\ttype:\t"+type+"\t"+declRef.str();
}
bool Variable::operator==(const Variable& b) {
  return type == b.type && name == b.name;
}

json Variable::dump() {
  json j;
  j["name"] = name;
  j["type"] = type;
  j["declRef"] = declRef.dump();
  j["type_sid"] = type_sid;
  return j;
}

Variable Variable::restore(json j, map<unsigned int, TypeBase*>& sid_type_map) {
  string name = j["name"].get<string>();
  string type = j["type"].get<string>();
  Pos declRef = Pos::restore(j["declRef"]);
  Variable v(type, name);
  j["type_sid"].get_to(v.type_sid);
  v.type_ptr = sid_type_map[v.type_sid];
  v.declRef = declRef;
  return v;
}




void Brick::print() {
  cout << "===========================================" << endl;
  cout << code << endl;
  cout << "In Brick from " << start.str() << " to " << end.str() << "body" << bodyStart.str() << endl;
  cout << "PreContext:" << endl;
  for(int j=0;j<preContext.size();j++) {
    Variable v = preContext[j];
    cout << "\t" << v.str() << endl;
  }
  cout << "PostContext:" << endl;
  for(int j=0;j<postContext.size();j++) {
    Variable v = postContext[j];
    cout << "\t" << v.str() << endl;
  }
}

// 将当前语句中的 rename_map[0] 置换为 rename_map[1]
// 注：一个 brick 可能包含多行代码
string Brick::rename(map<int, string> rename_map) {
  vector<string> lines = split(code, '\n');
  for(int l=0;l<lines.size();l++) {
    string line = lines[l];
    // 寻找定义在该行的变量,按列号升序
    vector<Variable*> decls;
    for(int j=0;j<preContext.size();j++) {
      Variable* v = &(preContext[j]);
      // 搜集在该行的变量
      if(v->declRef.line == start.line + l) {
        decls.push_back(v);
        // 插入排序
        int cur = decls.size() - 1;
        while(cur > 0 && decls[cur]->declRef.column < decls[cur-1]->declRef.column) {
          Variable* tmp = decls[cur];
          decls[cur] = decls[cur-1];
          decls[cur-1] = tmp;
        }
      }
    }
    // cout << "In line " << l << "have decl:" << decls.size() << endl;
    // 拆分字符串
    vector<string> parts;
    int end = 0;
    // cout << "rename debug: ==========" << endl;
    // line = "#" + line;
    // cout << line << endl;
    for(int i=0;i<decls.size();i++) {
      Variable* decl = decls[i];
      // -1 是因为ast分析的时候从1开始计数
      // 在这里如果是多行的话会出错,因为start.column只对第一行生效,1 表示没有空行
      int first_line_column = (l == 0) ? start.column : 1;
      // 这里可能出问题失败，我们直接跳过即可
      if(end > line.size()) {
        cout << "out_of_range error !" << endl;
        return "";
      }
      string a = line.substr(end, (decl->declRef.column-first_line_column)-end);
      // cout << a << endl;
      parts.push_back(a);
      parts.push_back(rename_map[decl->declRef.column]);
      // parts.push_back(line.substr(decl->scopeBegin.column, decl->name.size()));
      end = (decl->declRef.column-first_line_column) + decl->name.size();
      // cout << "end: " << end << endl;
    }
    if(end > line.size()) {
      cout << "out_of_range error !" << endl;
      return "";
    }
    parts.push_back(line.substr(end, line.size() - end));
    lines[l] = "";
    for(string p : parts) lines[l] += p;
    // cout << lines[l] << endl;
  }
  return join('\n', lines);
}

json Brick::dump() {
  json j;
  j["code"] = code;
  j["start"] = start.dump();
  j["end"] =  end.dump();
  j["bodyStart"] = bodyStart.dump();
  j["type"] = type;
  j["id"] = id;
  json preContext_j;
  for(Variable v : preContext) {
    preContext_j.push_back(v.dump());
  }
  j["preContext"] = preContext_j;
  json postContext_j;
  for(Variable v : postContext) {
    postContext_j.push_back(v.dump());
  }
  j["postContext"] = postContext_j;
  json preFunc_j;
  for(Signature s : preFunc) {
    preFunc_j.push_back(s.dump());
  }
  j["preFunc"] = preFunc_j;
  return j;
}

Brick Brick::restore(json j, map<unsigned int, TypeBase*>& sid_type_map) {
  // cout << j.dump() << endl;
  string code, type;
  j["code"].get_to(code);
  j["type"].get_to(type);
  // cout << code << endl;
  Pos start = Pos::restore(j["start"]);
  Pos end = Pos::restore(j["end"]);
  Pos bodyStart = Pos::restore(j["bodyStart"]);
  Brick b(start, end);
  b.code = code;
  b.type = type;
  j["id"].get_to(b.id);
  json preContext = j["preContext"];
  json postContext = j["postContext"];
  json preFunc = j["preFunc"];
  for (json::iterator it = preContext.begin(); it != preContext.end(); ++it) {
    Variable v = Variable::restore(*it, sid_type_map);
    b.preContext.push_back(v);
  }
  for (json::iterator it = postContext.begin(); it != postContext.end(); ++it) {
    Variable v = Variable::restore(*it, sid_type_map);
    b.postContext.push_back(v);
  }
  for (json::iterator it = preFunc.begin(); it != preFunc.end(); ++it) {
    Signature s = Signature::restore(*it, sid_type_map);
    b.preFunc.push_back(s);
  }
  return b;
}

vector<int> randRecord;
int randAction = -1;

// -1 表示直接随机，如果是非 -1 的值表示读取 randRecord
int getRand(int n) {
  assert(n != 0);
  int r;
  if(randAction == -1) {
      r = rand() % n;
      randRecord.push_back(r);
  } else {
    r = randRecord[randAction++];
  }
  // cout << "n: " << n << " r: " << r << endl;
  return r;
  // 通过更改 rand 结果保证稳定运行，方便debug
  // return n-1;
}


vector<string> split(string code, char sep) {
  vector<string> res;
  int sp = 0,fp=0;
  while(fp < code.length()){
      fp = code.find(sep,sp);
      res.push_back(code.substr(sp, fp-sp));
      sp = fp + 1;
  }
  return res;
}

string join(char c, vector<string> src){
    string res = "";
    if(src.size() == 0) return res;

    vector<string>::iterator it = src.begin();
    res += *it;
    
    for(it++;it!=src.end();it++){
        res += c;
        res += *it;
    }
    return res;
}

// 后续的参数必须使用string.c_str()
string path_join(int num, ...) {
  va_list args;  //定义一个可变参数列表
	va_start(args, num);  //初始化args指向强制参数arg的下一个参数
  string path;
  while(num > 0) {
    char *arg;
    arg = va_arg(args, char*);
    // 去除后缀的 /
    int len = strlen(arg);
    while(len > 0 && arg[len-1] == '/') {
      len--;
    }
    string tmp(arg);
    path += tmp.substr(0, len);
    if(num != 1) path += "/";
    num--;
  }
  return path;
}

// discard：不一定需要不重复
// 从 m 数种随机选 n 个不重复的数字
vector<int> getUniNfromM(int n, int m) {
  assert(n > m);
  vector<int> res;
  for(int i=0;i<m;i++) {
    res.push_back(i);
  }
  // 第i个元素和前面i-1个元素中的某一个互换位置
  for(int i=m-1;i>0;i--) {
    int pivot = getRand(i);
    int tmp = res[i];
    res[i] = res[pivot];
    res[pivot] = tmp;
  }
  res = vector<int>(res.begin(), res.begin()+n);
  return res;
}

// 从 M 中选取 N 个可能重复的数字
vector<int> getNfromM(int n, int m) {
  vector<int> res;
  for(int i=0;i<n;i++) {
    res.push_back(getRand(m));
  }
  return res;
}


// 从preContext的变量名映射到context的变量名
vector<string> getMap(vector<Variable> context, vector<Variable> preContext) {
  // map<string, string> res;
  vector<string> res;
  // 搜寻相同的类型的变量
  map<string, vector<string>> context_map; 
  // 构建当前上下文中 type 到 变量名的映射
  for(Variable v : context) {
    if(context_map.find(v.type) == context_map.end()) {
      vector<string> tmp;
      tmp.push_back(v.name);
      context_map[v.type] = tmp;
    } else {
      context_map[v.type].push_back(v.name);
    }
  }
  // 构建 pre_context 中 type 到 变量名的映射
  map<string, vector<string>> preContext_map;
  for(Variable v : preContext) {
    if(preContext_map.find(v.type) == preContext_map.end()) {
      vector<string> tmp;
      tmp.push_back(v.name);
      preContext_map[v.type] = tmp;
    } else {
      preContext_map[v.type].push_back(v.name);
    }
  }
  // 对每一个pre_context中的类型，从当前上下文中挑选pre_context需要数量的的变量
  for(map<string, vector<string>>::iterator it=preContext_map.begin();it!=preContext_map.end();it++) {
    string type = it->first;
    vector<string> same_type_vars_pre = it->second;
    vector<string> same_type_vars_context = context_map[type];
    vector<int> rename_map = getNfromM(same_type_vars_pre.size(), same_type_vars_context.size());
    // 对相同类型的每一个变量
    for(int i=0;i<rename_map.size();i++) {
      // pre_context 的第 i 个变量对应 cur_context 的第 rename[i] 个变量
      // res[same_type_vars_pre[i]] =  same_type_vars_context[rename_map[i]];
      res.push_back(same_type_vars_context[rename_map[i]]);
    }
  }
  return res;
}

bool isMatch(TypeBase* p, TypeBase* c) {
  // 不考虑指针可以转换为数组，如果去语法糖后的类型不一致则直接不 match
  if(p->getDesugaredTypePtr()->eType != c->getDesugaredTypePtr()->eType) {
    return false;
  }
  switch(p->getDesugaredTypePtr()->eType) {
    case eStruct:
    case eFunctionPointer:
    case eUnion: {
      return p->name == c->name;
    }
    case eArray:
    case ePointer: {
      return isMatch(p->ptr_type, c->ptr_type);
    }
    case eSimple: {
      // 基本类型都除了 void 都可以隐式转换，所以直接返回 true
      SimpleImpl* p_simple = (SimpleImpl*) p->getDesugaredTypePtr();
      SimpleImpl* c_simple = (SimpleImpl*) c->getDesugaredTypePtr();
      if(p_simple->UnqualifiedType == "void" && c_simple->UnqualifiedType == "void") return true;
      if(p_simple->UnqualifiedType == "void" || c_simple->UnqualifiedType == "void") return false;
      return true; 
    }
    default: {
      return false;
    }
  }
}

// 返回 parent 的变量是否可以涵盖 child 的变量，如果可以renameMap 的大小为 postContext 的大小，否则为 0
map<int, string> isSubset(vector<Variable> parent, vector<Variable> child) {
  // vector<string> renameMap;
  // 行号对应的名称
  map<int, string> renameMap;
  for(Variable c : child) {
    vector<Variable> participants;
    for(Variable p : parent) {
      if(isMatch(p.type_ptr, c.type_ptr)) {
        participants.push_back(p);
      }
    }
    // 如果有一个不满足，则直接失败
    if(participants.size() == 0) {
      renameMap.clear();
      return renameMap;
    } else {
      int rand = getRand(participants.size());
      renameMap[c.declRef.column] = participants[rand].name;
    }
  }
  return renameMap;
}

void writeFileJson(string path, json j) {
  ofstream os;
  os.open(path, ios::out);
  if (!os.is_open())
    cout << "error：can not find or create the file" << endl;
  os << j.dump(2);
  os.close();
}

vector<string> getFiles(string dir) {
  DIR* d = opendir(dir.c_str());
  if (d == NULL) {
    printf("d == NULL");
  }
  vector<string> res;
  struct dirent* entry;
  while ( (entry=readdir(d)) != NULL) {
    string filename(entry->d_name);
    res.push_back(filename);
  }

  closedir(d);
  return res;
}


json Signature::dump() {
  json j;
  j["start"] = start.dump();
  j["sig"] = sig;
  j["name"] = name;
  j["returnValue"] = returnValue.dump();
  json params_json;
  for(Variable v : params) {
    params_json.push_back(v.dump());
  }
  j["params"] = params_json;
  return j;
}

Signature Signature::restore(json j, map<unsigned int, TypeBase*>& sid_type_map) {
  Signature s;
  string sig, name;
  j["sig"].get_to(sig);
  j["name"].get_to(name);
  s.sig = sig;
  s.name = name;
  s.start = Pos::restore(j["start"]);
  s.returnValue = Variable::restore(j["returnValue"], sid_type_map);
  json params = j["params"];
  for (json::iterator it = params.begin(); it != params.end(); ++it) {
    Variable v = Variable::restore(*it, sid_type_map);
    s.params.push_back(v);
  }
  return s;
}

// bool Struct::operator== (const Struct& s) {
//   // todo: 偷个懒，先不处理field 不同的情况
//     return s.name == name;
// }

string getFuncPointerStr(string type, string name) {
  int index = type.find("(*)");
  string tmp = "";
  string partA = type.substr(0, index+2);
  string partB = type.substr(index+2, type.size() - partA.size());
  tmp += partA;
  tmp += name;
  tmp += partB;
  return tmp;
}

string getArrayStr(string type, string name) {
  int index = type.find("[");
  string tmp = "";
  string partA = type.substr(0, index);
  string partB = type.substr(index, type.size() - partA.size());
  tmp += partA;
  tmp += " ";
  tmp += name;
  tmp += partB;
  return tmp;
}

// string Struct::str() {
//   string res = isUnion ? "union " : "struct ";
//   res += name;
//   res += " {\n";
//   for(Variable v : fields) {
//     string tmp = "\t";
//     // uname
//     // 数组
//     Array* a = getArrayInfo(v.type);
//     if(a->isValid()) {
//       tmp += a->name + " " + name + "[" + to_string(a->size) + "];\n";
//       res += tmp;
//       continue;
//     }
//     // 函数指针
//     int index = v.type.find("(*)");
//     if(index != v.type.npos) {
//       tmp += getFuncPointerStr(v.type, v.name);
//       tmp += ";\n";
//       res += tmp;
//       continue;
//     }
//     // 正常
//     tmp += v.type;
//     tmp += " ";
//     tmp += v.name;
//     tmp += ";\n";
//     res += tmp;
//   }
//   res += "};\n";
//   return res;
// }

// json Struct::dump() {
//   json j;
//   j["name"] = name;
//   j["isUnion"] = isUnion;
//   json field_json;
//   for(Variable v : fields) {
//     field_json.push_back(v.dump());
//   }
//   j["fields"] = field_json;
//   return j;
// }

// Struct Struct::restore(json j) {
//   Struct s;
//   string name;
//   j["name"].get_to(name);
//   j["isUnion"].get_to(s.isUnion);
//   s.name = name;
//   json fields = j["fields"];
//   // for (json::iterator it = fields.begin(); it != fields.end(); ++it) {
//   //   Variable v = Variable::restore(*it);
//   //   s.fields.push_back(v);
//   // }
//   return s;
// }


// string Typedef::str() {
//   string res = "typedef ";
//   if(isUnname) {
//     // 去掉最后的 ； 和 \n
//     res += type.substr(0, type.size()-2);
//     res += " ";
//     res += name;
//     res += ";\n";
//     return res;
//   }
//   // 数组
//   Array* a = getArrayInfo(type);
//   if(a->isValid()) {
//     res += a->name + " " + name + "[" + to_string(a->size) + "];\n";
//     return res;
//   }
//   // 函数指针
//   int index = type.find("(*)");
//   if(index != type.npos) {
//     res += getFuncPointerStr(type, name);
//     res += ";\n";
//     return res;
//   }
//   // 正常
//   res += type;
//   res += " ";
//   res += name;
//   res += ";\n";
//   return res;
// }

// json Typedef::dump() {
//   json j;
//   j["name"] = name;
//   j["type"] = type;
//   j["isUnname"] = isUnname;
//   return j;
// }

// Typedef Typedef::restore(json j) {
//   string name, type;
//   int isUnname;
//   j["name"].get_to(name);
//   j["type"].get_to(type);
//   j["isUnname"].get_to(isUnname);
//   Typedef t(name, type);
//   t.isUnname = isUnname;
//   return t;
// }

// 去除type 的前置 const 和 后置*, 以及前后的空格空格
string removeQualOfType(string type) {
  // 这里必须是 const空格，防止变量名中有 const
  vector<string> qualifieds = {"*", "const "};
  for(string qual : qualifieds) {
    int index = type.find(qual, 0);
    while(index != type.npos) {
      type.replace(index, qual.size(), "");
      index = type.find(qual, 0);
    }
  }
  // 去除变量中的数组描述,只保留前面的类型
  int index = type.find("[");
  if(index != type.npos) {
    type = type.substr(0, index);
  }
  // 去除先后空格
  int start = 0, end = type.size()-1;
  while(type[start] == ' ') start++;
  while(type[end] == ' ') end--;
  type = type.substr(start, end-start+1);
  return type;
}

// string DependencyNode::print(bool printMyself) {
//   string res = "";
//   // 先遍历 typedef 还是 structs 没有区别
//   for(DependencyNode* td : typedef_dependency) {
//     res += td->print(true);
//     res += "\n";
//   }
//   for(DependencyNode* s : struct_dependency) {
//     res += s->print(true);
//     res += "\n";
//   }
//   if(printMyself) res += str();
//   res += "\n";
//   return res;
// }

// void DependencyNode::clean(set<string>& used_set) {
//   for(DependencyNode* s : struct_dependency) s->clean(used_set);
//   for(DependencyNode* td : typedef_dependency) td->clean(used_set);
//   if(used_set.find(name) != used_set.end()) {
//     used_set.erase(name);
//   }
//   return;
// }

vector<string> findSizeofType(string code) {
    regex pattern("sizeof\\((\\w+)\\)");
    smatch m;
    auto pos = code.cbegin();
    auto end = code.cend();
    vector<string> res;
    for (; regex_search(pos, end, m, pattern); pos = m.suffix().first)
    {
        cout << "find sizeof " << m.str(1) << std::endl;
        res.push_back(m.str(1));
    }
    return res;
}



Array* getArrayInfo(string type) {
  int left = type.find('[');
  if(left == type.npos) {
    return new Array("", -1);
  }
  int right = type.find(']');
  string name = type.substr(0, left);
  int size;
  // "[]"
  if(right == left+1) size = -1;
  else size = stoi(type.substr(left+1, right-left-1));
  return new Array(name, size);
}
