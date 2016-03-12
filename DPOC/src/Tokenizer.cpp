#include <stdexcept>

#include "Error.h"
#include "Utility.h"
#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string& line, int lineNumber)
  : m_line(line),
    m_currentIndex(0),
    m_lineNumber(lineNumber)
{
}

std::vector<std::string> Tokenizer::tokenize()
{
  m_tokens.clear();
  m_currentIndex = 0;
  m_currentToken.clear();

  while (!atEnd())
  {
    char c = get();

    if (c == '\"')
    {
      parseString();
      flush();
    }
    else if (!std::isspace(c))
    {
      parseAtom();
      flush();
    }

    advance();
  }

  return m_tokens;
}

void Tokenizer::flush()
{
  m_tokens.push_back(m_currentToken);
  m_currentToken.clear();
}

bool Tokenizer::hasNext() const
{
  return m_currentIndex < (m_line.size() - 1);
}

bool Tokenizer::atEnd() const
{
  return m_currentIndex >= m_line.size();
}

char Tokenizer::get() const
{
  if (!atEnd())
  {
    return m_line[m_currentIndex];
  }

  parseError("trying to read outside bounds.");
  return 0;
}

void Tokenizer::advance()
{
  m_currentIndex++;
}

void Tokenizer::consume()
{
  m_currentToken += get();
  advance();
}

void Tokenizer::parseAtom()
{
  bool bracketParsing = false;

  while (!atEnd())
  {
    char c = get();

    if (!bracketParsing && std::isspace(c))
    {
      if (!hasNext() && bracketParsing)
      {
        parseError("Unmatched '[' character!");
      }

      break;
    }
    else if (c == '[')
    {
      bracketParsing = true;
    }
    else if (c == ']')
    {
      if (!bracketParsing)
      {
        parseError("Unexpected ']' character!");
      }

      bracketParsing = false;
    }

    consume();
  }
}

void Tokenizer::parseString()
{
  // Skip leading quotation.
  if (get() == '\"')
  {
    advance();
  }

  while (!atEnd())
  {
    char c = get();

    if (c == '\"')
    {
      break;
    }
    else if (c == '\\')
    {
      advance();
    }

    consume();
  }
}

void Tokenizer::parseError(const std::string& message) const
{
  CRASH("Parse error {line %d}: %s", m_lineNumber, message.c_str());
}
