#pragma once
#include <cctype>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

namespace hy {

enum class TokenType {
  Exit,        // "exit"
  Let,         // "let"
  Ident,       // identifier
  IntLiteral,  // e.g. 123
  Semi,        // ';'
  OpenParen,   // '('
  CloseParen,  // ')'
  Equals       // '='
};

struct Token {
  TokenType type{};
  // Only one of these has meaning depending on token type:
  std::optional<long long> int_value;
  std::optional<std::string> str_value;
};

class Tokenizer {
public:
  explicit Tokenizer(std::string source) : m_source(std::move(source)) {}

  std::vector<Token> tokenize() {
    std::vector<Token> out;
    m_index = 0;

    auto push_simple = [&](TokenType t) { out.push_back(Token{t, std::nullopt, std::nullopt}); };

    while (auto ch = peek(0)) {
      const char c = *ch;

      // whitespace
      if (std::isspace(static_cast<unsigned char>(c))) { consume(); continue; }

      // single-char tokens
      if (c == ';') { consume(); push_simple(TokenType::Semi); continue; }
      if (c == '(') { consume(); push_simple(TokenType::OpenParen); continue; }
      if (c == ')') { consume(); push_simple(TokenType::CloseParen); continue; }
      if (c == '=') { consume(); push_simple(TokenType::Equals); continue; }

      // identifier / keyword
      if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        std::string buf;
        buf.push_back(consume());
        while (auto c2 = peek(0)) {
          if (std::isalnum(static_cast<unsigned char>(*c2)) || *c2 == '_') buf.push_back(consume());
          else break;
        }
        if (buf == "exit")  { push_simple(TokenType::Exit);  continue; }
        if (buf == "let")   { push_simple(TokenType::Let);   continue; }
        out.push_back(Token{TokenType::Ident, std::nullopt, buf});
        continue;
      }

      // integer literal
      if (std::isdigit(static_cast<unsigned char>(c))) {
        std::string buf;
        buf.push_back(consume());
        while (auto c2 = peek(0)) {
          if (std::isdigit(static_cast<unsigned char>(*c2))) buf.push_back(consume());
          else break;
        }
        long long v = 0;
        try { v = std::stoll(buf); }
        catch (...) {
          std::cerr << "Invalid integer: " << buf << "\n";
          std::exit(EXIT_FAILURE);
        }
        out.push_back(Token{TokenType::IntLiteral, v, std::nullopt});
        continue;
      }

      std::cerr << "Unexpected character: '" << c << "'\n";
      std::exit(EXIT_FAILURE);
    }

    return out;
  }

private:
  std::optional<char> peek(size_t offset = 0) const {
    const size_t i = m_index + offset;
    if (i < m_source.size()) return m_source[i];
    return std::nullopt;
  }

  char consume() {
    return m_source[m_index++];
  }

  std::string m_source;
  size_t m_index{0};
};

} // namespace hy
