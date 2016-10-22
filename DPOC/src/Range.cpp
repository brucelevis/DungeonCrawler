#include "Range.h"

Range::Range()
  : m_index(0),
    m_start(0),
    m_end(0),
    m_min(0),
    m_max(0),
    m_rangeLength(0)
{
}

Range::Range(int min, int max, int rangeLength)
  : m_index(0),
    m_start(0),
    m_end(rangeLength),
    m_min(min),
    m_max(max),
    m_rangeLength(rangeLength)
{
}

void Range::reset()
{
  m_index = 0;
  m_start = 0;
  m_end = m_rangeLength;
}

void Range::addIndex(int num, WrapMode wrapMode)
{
  m_index += num;

  fixWrap(wrapMode);

  if (m_index >= m_end)
  {
    m_start++;
    m_end++;
  }
}

void Range::subIndex(int num, WrapMode wrapMode)
{
  m_index -= num;

  fixWrap(wrapMode);

  if (m_index < m_start)
  {
    m_start--;
    m_end--;
  }
}

void Range::fixWrap(WrapMode wrapMode)
{
  if (m_index >= m_max)
  {
    if (wrapMode == NO_WRAP)
    {
      m_index = m_max - 1;
    }
    else
    {
      m_index = m_min;
    }
  }
  else if (m_index < m_min)
  {
    if (wrapMode == NO_WRAP)
    {
      m_index = m_min;
    }
    else
    {
      m_index = m_max - 1;
    }
  }
}

int Range::getIndex() const
{
  return m_index;
}

int Range::getStart() const
{
  return m_start;
}

int Range::getEnd() const
{
  return m_end;
}

int Range::getMin() const
{
  return m_min;
}

int Range::getMax() const
{
  return m_max;
}
