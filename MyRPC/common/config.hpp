#pragma once

#include <stdlib.h>

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "strings.hpp"

namespace Common {
class Config {
 public:
  void Dump(std::function<void(const std::string&, const std::string&, const std::string&)> deal) {
    auto iter = cfg_.begin();
    while (iter != cfg_.end()) {
      auto kvIter = iter->second.begin();
      while (kvIter != iter->second.end()) {
        deal(iter->first, kvIter->first, kvIter->second);
        ++kvIter;
      }
      ++iter;
    }
  }
  bool Load(std::string fileName) {
    if (fileName == "") return false;
    std::ifstream in;
    std::string line;
    in.open(fileName.c_str());
    if (not in.is_open()) return false;
    while (getline(in, line)) {
      std::string section, key, value;
      if (not parseLine(line, section, key, value)) {
        continue;
      }
      setSectionKeyValue(section, key, value);
    }
    return true;
  }
  void GetStrValue(std::string section, std::string key, std::string& value, std::string defaultValue) {
    if (cfg_.find(section) == cfg_.end()) {
      value = defaultValue;
      return;
    }
    if (cfg_[section].find(key) == cfg_[section].end()) {
      value = defaultValue;
      return;
    }
    value = cfg_[section][key];
  }
  void GetIntValue(std::string section, std::string key, int64_t& value, int64_t defaultValue) {
    if (cfg_.find(section) == cfg_.end()) {
      value = defaultValue;
      return;
    }
    if (cfg_[section].find(key) == cfg_[section].end()) {
      value = defaultValue;
      return;
    }
    value = atol(cfg_[section][key].c_str());
  }

 private:
  void setSectionKeyValue(std::string& section, std::string& key, std::string& value) {
    if (cfg_.find(section) == cfg_.end()) {
      std::map<std::string, std::string> kvMap;
      cfg_[section] = kvMap;
    }
    if (key != "" && value != "") cfg_[section][key] = value;
  }
  bool parseLine(std::string& line, std::string& section, std::string& key, std::string& value) {
    static std::string curSection = "";
    std::string nodes[2] = {"#", ";"};  //去掉注释的内容
    for (int i = 0; i < 2; ++i) {
      std::string::size_type pos = line.find(nodes[i]);
      if (pos != std::string::npos) line.erase(pos);
    }
    Strings::trim(line);
    if (line == "") return false;
    if (line[0] == '[' && line[line.size() - 1] == ']') {
      section = line.substr(1, line.size() - 2);
      Strings::trim(section);
      curSection = section;
      return false;
    }
    if (curSection == "") return false;
    bool isKey = true;
    for (size_t i = 0; i < line.size(); ++i) {
      if (line[i] == '=') {
        isKey = false;
        continue;
      }
      if (isKey) {
        key += line[i];
      } else {
        value += line[i];
      }
    }
    section = curSection;
    Strings::trim(key);
    Strings::trim(value);
    return true;
  }

 private:
  std::map<std::string, std::map<std::string, std::string>> cfg_;
};  // ini格式配置文件的读取
}  // namespace Common