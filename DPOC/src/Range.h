#ifndef RANGE_H_
#define RANGE_H_


struct Range
{
  enum WrapMode
  {
    NO_WRAP,
    WRAP
  };

  Range();
  Range(int min, int max, int rangeLength);

  void reset();
  void moveTo(int targetIndex);

  void addIndex(int num, WrapMode wrapMode);
  void subIndex(int num, WrapMode wrapMode);

  int getIndex() const;

  int getStart() const;
  int getEnd() const;

  int getMin() const;
  int getMax() const;

  int getRangeLength() const;
private:
  int size() const;
  void fixWrap(WrapMode wrapMode);
private:
  int m_index;
  int m_start, m_end;
  int m_min, m_max;
  int m_rangeLength;
};


#endif /* RANGE_H_ */
