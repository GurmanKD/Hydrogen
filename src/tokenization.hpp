#pragma once
#include <cctype>
#include <optional>
#include <string>
#include <vector>

enum class TokenType { Exit, IntLiteral, Semicolon };

struct Token {
  TokenType type;
  std::optional<std::string> value; // for IntLiteral
};

class Tokenizer {
public:
  explicit Tokenizer(std::string source) : m_source(std::move(source)) {}

  std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    std::string buf;

    while (peek().has_value()) {
      char c = *peek();

      // whitespace
      if (std::isspace(static_cast<unsigned char>(c))) { consume(); continue; }

      // identifier/keyword
      if (std::isalpha(static_cast<unsigned char>(c))) {
        buf.clear();
        buf.push_back(consume());
        while (peek().has_value() &&
               std::isalnum(static_cast<unsigned char>(*peek()))) {
          buf.push_back(consume());
        }
        if (buf == "exit") {
          tokens.push_back({TokenType::Exit, std::nullopt});
        } else {
          throw std::runtime_error("Unknown identifier: " + buf);
        }
        continue;
      }

      // integer literal
      if (std::isdigit(static_cast<unsigned char>(c))) {
        buf.clear();
        buf.push_back(consume());
        while (peek().has_value() &&
               std::isdigit(static_cast<unsigned char>(*peek()))) {
          buf.push_back(consume());
        }
        tokens.push_back({TokenType::IntLiteral, buf});
        continue;
      }

      // semicolon
      if (c == ';') { consume(); tokens.push_back({TokenType::Semicolon, std::nullopt}); continue; }

      throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    // allow re-use if needed
    m_index = 0;
    return tokens;
  }

private:
  [[nodiscard]] std::optional<char> peek(int ahead = 1) const {
    // IMPORTANT: strictly '>' (not >=) to avoid off-by-one
    if (m_index + static_cast<size_t>(ahead) > m_source.size()) return std::nullopt;
    return m_source.at(m_index + ahead - 1);
  }

  char consume() { return m_source.at(m_index++); }

  std::string m_source;
  std::size_t m_index = 0;
};
