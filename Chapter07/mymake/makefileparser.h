#pragma once
#include <assert.h>

#include <iostream>
#include <string>
#include <vector>

namespace MyMake {
enum ParserStatus {
  INIT = 1,        // 初始化状态
  COLON = 2,       // 冒号
  IDENTIFIER = 3,  // 标识符（包括关键字）-> .PHONY mymake.cpp mymake.o
  TAB = 4,         // tab键
  CMD = 5,         // 要执行的命令
};

typedef int TokenType;

typedef struct Token {
  int32_t line_pos;      // 在行中的位置
  int32_t line_number;   // token在makefile文件中的第几行
  std::string text;      // token的文本内容
  TokenType token_type;  // token类型，使用ParserStatus枚举赋值
  void Print() {
    std::cout << "line_number[" << line_number << "],line_pos[" << line_pos << "],token_type[" << GetTokenTypeStr()
              << "],text[" << text << "]" << std::endl;
  }
  std::string GetTokenTypeStr() {
    if (token_type == COLON) {
      return "COLON";
    }
    if (token_type == IDENTIFIER) {
      return "IDENTIFIER";
    }
    if (token_type == TAB) {
      return "TAB";
    }
    if (token_type == CMD) {
      return "CMD";
    }
    assert(0);
  }
} Token;

class Parser {
 public:
  bool ParseToToken(std::string file_name, std::vector<std::vector<Token>>& tokens);

 private:
  void parseLine(std::string line, int32_t line_number, std::string& token, ParserStatus& parseStatus,
                 std::vector<Token>& tokens_list);
};
}  // namespace MyMake