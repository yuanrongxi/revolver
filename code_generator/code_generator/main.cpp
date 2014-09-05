//#include "strtk.hpp"
#include <iostream>
#include <stdint.h>
#include <direct.h>
#include <regex>
#include <fstream>
#include <set>
#include <map>
#include <sstream>

using namespace std;

string srv_prefix;
string marco_prefix;
string output_path = "./output";

string marco_var_prefix = "$var_prefix$";
static set<string> _variables_from_template;
static map<string, string> _kv_variables;

bool test_replace_pattern();

string ltrim(const string& str, const char* delimiter = " /t/n/r") {
  string t = str;
  t.erase(0, t.find_first_not_of(delimiter));
  return t;
}

string rtrim(const string& str, const char* delimiter = " /t/n/r") {
  string t = str;
  t.erase(t.find_last_not_of(delimiter) + 1);
  return t;
}

string trim(const string& str, const char* delimiter = " /t/n/r") {
  return ltrim(rtrim(str, delimiter), delimiter);
}

int32_t generate_code(const char* file_name);
int32_t analyze_template(const char* file_name);

int main(int argc, char* argv[]) {

  
  std::string srv_name = "Sample";
  /*std::string srv_type, srv_name;
  std::cout << "input server name:";
  std::cin >> srv_name;
  std::cout << "input server type:";
  std::cin >> srv_type;*/

  std::string file_name_prefix, val;
  // analyze the server name generate file name prefix.
  uint32_t last_pos = 0;
  for(uint32_t i = 0; i < srv_name.size(); ++ i) {
    char ch = srv_name[i];
    if (isupper(ch) && i) {
      val = srv_name.substr(last_pos, i - last_pos);
      last_pos = i;
      file_name_prefix += val;
      file_name_prefix += '_';
    }
  }
  val = srv_name.substr(last_pos, std::string::npos);
  file_name_prefix += val;
  std::transform(file_name_prefix.begin(), file_name_prefix.end(), file_name_prefix.begin(), ::tolower);
  marco_prefix = file_name_prefix;
  std::transform(marco_prefix.begin(), marco_prefix.end(), marco_prefix.begin(), ::toupper);
  std::cout << file_name_prefix << std::endl;
  std::cout << marco_prefix << std::endl;

  _kv_variables["marco_prefix"] = marco_prefix;
  _kv_variables["var_prefix"] = file_name_prefix;
  _kv_variables["file_prefix"] = file_name_prefix;
  _kv_variables["class_prefix"] = srv_name;
  /*std::transform(srv_name.begin(), srv_name.end(), srv_name.begin(), ::tolower);
  _kv_variables["func_prefix"] = srv_name;*/

  _variables_from_template.clear();

  std::ifstream file_list;
  file_list.open("./template/list.txt");
  if (!file_list.is_open()) {
    std::cout << "failed to open index file" << std::endl;
    return -1;
  }
  char rd_buf[100];
  while (file_list.getline(rd_buf, 100)) {
    cout << rd_buf << endl;
    string tpl_path = "./template/";
    tpl_path += rd_buf;
    cout << "analyzing file: " << tpl_path << endl;
    analyze_template(tpl_path.c_str());

    // generate the file
    generate_code(tpl_path.c_str());
  }

  // scan the variables
  auto itor = _variables_from_template.begin();
  while (itor != _variables_from_template.end()) {
    cout << *itor << endl;
    ++ itor;
  }

  // generate files.

  goto l_end;

 l_end:
  std::cout << "......" << std::endl;
 
  system("pause");
  return 0;
}

int32_t analyze_template(const char* file_name) {
  std::ifstream in;
  in.open(file_name);

  if (!in.is_open()) {
    std::cout << "failed to open " << file_name << std::endl;
    return -1;
  }

  // get template file length.
  int64_t tmp = in.tellg();
  in.seekg(0, ios_base::end);
  int64_t length = in.tellg();
  in.seekg(0, ios_base::beg);

  // read all data into buf
  std::string buf((std::istreambuf_iterator<char>(in)),
    std::istreambuf_iterator<char>());

  std::regex pattern("##.*?#");
  sregex_iterator it(buf.begin(), buf.end(), pattern);
  sregex_iterator it_end;

  while (it != it_end) {
    std::string var = it->str();
    var = trim(var, "#");
    //std::cout << it->str() << endl;
    _variables_from_template.insert(var);
    ++it;
  }
  return 0;
}

int32_t generate_code(const char* file_name) {
  std::ifstream in;
  in.open(file_name);

  if (!in.is_open()) {
    std::cout << "failed to open " << file_name << std::endl;
    return -1;
  }

  // get template file length.
  int64_t tmp = in.tellg();
  in.seekg(0, ios_base::end);
  int64_t length = in.tellg();
  in.seekg(0, ios_base::beg);

  // read all data into buf
  std::string buf((std::istreambuf_iterator<char>(in)),
    std::istreambuf_iterator<char>());

  auto itor = _kv_variables.begin();
  // generate the file name
  string name = file_name;
  while (itor != _kv_variables.end()) {
    ostringstream os;
    os << "##" << itor->first << "#";
    string str = os.str();
    std::regex pattern(str.c_str());
    name = regex_replace(name, pattern, itor->second);
    ++ itor;
  }
  cout << "generate file: " << name << endl;

  itor = _kv_variables.begin();
  while (itor != _kv_variables.end()) {
    ostringstream os;
    os << "##" << itor->first << "#";
    string str = os.str();

    std::regex pattern(str.c_str());
    buf = regex_replace(buf, pattern, itor->second);
    ++ itor;
  }

  // save to file
  ofstream of;
  of.open(name.c_str());
  if (of.is_open()) {
    of << buf;
    of.close();
  }
  cout << buf << endl;
  return 0;
}
//bool test_replace_pattern() {
//  typedef std::pair<std::string, std::string> sp_type;
//  static const sp_type test[] = {
//    sp_type("a" ,   "x"),
//    sp_type("ab" ,  "xy"),
//    sp_type("abc" , "xyz"),
//    sp_type("a" ,  "xy"),
//    sp_type("b" ,  "xy"),
//    sp_type("c" ,  "xy"),
//    sp_type("ab" ,   "x"),
//    sp_type("bc" ,   "x"),
//    sp_type("ca" ,   "x"),
//    sp_type("ab" , "xyz"),
//    sp_type("bc" , "xyz"),
//    sp_type("ca" , "xyz"),
//    sp_type("abc" ,   "x"),
//    sp_type("bca" ,   "x"),
//    sp_type("cab" ,   "x"),
//    sp_type("abca" ,   "x"),
//    sp_type("bcab" ,   "x"),
//    sp_type("cabc" ,   "x"),
//    sp_type("abc" ,    ""),
//    sp_type("ijk" , "mno")
//  };
//
//  static const std::size_t test_size = sizeof(test) / sizeof(sp_type);
//
//
//  static const std::string base[] = {
//    "",
//    "abcabcabcabcabc",
//    "yyabcabcabcabcabc",
//    "yyabcabcabcabcabckkk",
//    "yyabcabcabcabcabckkk",
//    "yabctabcabctabcabtckk",
//    "xyzxyzxyzxyzxyzxyzxyzxyz"
//    "abc"
//  };
//
//  static const std::size_t base_size = sizeof(base) / sizeof(std::string);
//
//  std::string result;
//  for(std::size_t i = 0; i < test_size; ++i) {
//    for(std::size_t j = 0; j < base_size; ++j) {
//      strtk::replace_pattern(base[j], test[i].first, test[i].second, result);
//      if(test[i].first != test[i].second) {
//        if(std::string::npos != result.find(test[i].first)) {
//          std::cout << "replace_pattern: s[" << base[j] << "] p[" << test[i].first << "] r[" << test[i].second << "]" << std::endl;
//          return false;
//        }
//      }
//    }
//  }
//
//  return true;
//}