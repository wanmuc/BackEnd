#include "makefileparser.h"

#include <fstream>

using namespace std;

namespace MyMake {
bool Parser::ParseToToken(string file_name, vector<vector<Token>>& tokens_list) {
  if (file_name == "") return false;
  ifstream in;
  in.open(file_name);
  if (not in.is_open()) return false;
  string token = "";
  string line;
  ParserStatus parse_status = INIT;
  int32_t line_number = 1;
  while (getline(in, line)) {
    line += '\n';
    vector<Token> tokens;
    parseLine(line, line_number, token, parse_status, tokens);
    line_number++;
    if (tokens.size() > 0) {
      tokens_list.push_back(tokens);
    }
  }
  return true;
}

void Parser::parseLine(std::string line, int32_t line_number, std::string& token, ParserStatus& parse_status,
                       std::vector<Token>& tokens) {
  auto getOneToken = [&token, &tokens, &parse_status, line_number](ParserStatus new_status, int32_t pos) {
    if (token != "") {
      Token t;
      t.line_pos = pos;
      t.line_number = line_number;
      t.text = token;
      t.token_type = parse_status;
      tokens.push_back(t);
    }
    token = "";
    parse_status = new_status;
  };
  int32_t pos = 0;
  for (size_t i = 0; i < line.size(); i++) {
    char c = line[i];
    if (parse_status == INIT) {
      if (c == ' ' || c == '\n') continue;
      if (c == '\t') {  // tab键
        parse_status = TAB;
      } else if (c == ':') {  // 冒号
        parse_status = COLON;
      } else {  // 其他字符
        parse_status = IDENTIFIER;
      }
      token = c;
      pos = i;
    } else if (parse_status == COLON) {
      if (c == ' ' || c == '\n') {
        getOneToken(INIT, pos);
        continue;
      }
      if (c == '\t') {
        getOneToken(TAB, pos);
      } else if (c == ':') {
        getOneToken(COLON, pos);
      } else {
        getOneToken(IDENTIFIER, pos);
      }
      token = c;
      pos = i;
    } else if (parse_status == IDENTIFIER) {
      if (c == ' ' || c == '\n') {
        getOneToken(INIT, pos);
        continue;
      }
      if (c == '\t') {
        getOneToken(TAB, pos);
        token = c;
        pos = i;
      } else if (c == ':') {
        getOneToken(COLON, pos);
        token = c;
        pos = i;
      } else {
        token += c;
      }
    } else if (parse_status == TAB) {
      if (isblank(c)) {  // 过滤掉tab键之后的空白符（space + tab）
        continue;
      }
      getOneToken(CMD, pos);  // 其他非空白符的部分就是命令
      token = c;
      pos = i;
    } else if (parse_status == CMD) {
      if (c == '\n') {
        getOneToken(INIT, pos);
      } else {
        token += c;
      }
    } else {
      assert(0);
    }
  }
}
}  // namespace MyMake