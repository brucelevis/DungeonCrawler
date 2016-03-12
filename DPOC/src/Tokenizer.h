#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <string>
#include <vector>

class Tokenizer
{
public:
  Tokenizer(const std::string& line, int lineNumber);

  std::vector<std::string> tokenize();
private:
  void flush();

  bool hasNext() const;

  bool atEnd() const;

  char get() const;

  void advance();

  void consume();

  void parseAtom();

  void parseString();

  void parseError(const std::string& message) const;
private:
  std::string m_line;
  std::string m_currentToken;
  size_t m_currentIndex;
  int m_lineNumber;
  std::vector<std::string> m_tokens;
};

#endif /* TOKENIZER_H_ */
