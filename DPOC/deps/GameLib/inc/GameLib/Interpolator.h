#ifndef GAMELIB_INTERPOLATOR_H
#define GAMELIB_INTERPOLATOR_H

namespace gamelib
{
  template <typename T>
  class Interpolator
  {
    static T lerp(const T& a, const T& b, float t)
    {
      return a + t * (b - a);
    }
  public:
    Interpolator()
      : m_start(), m_target(), m_total(0), m_timestep(0)
    {
    }

    Interpolator(const T& start, const T& target, float timestep)
      : m_start(start), m_target(target), m_total(0), m_timestep(timestep)
    {
    }

    void set(const T& start, const T& target, float timestep)
    {
      m_total = 0;
      m_current = start;
      m_start = start;
      m_target = target;
      m_timestep = timestep;
    }

    void update()
    {
      m_total += m_timestep;
      m_current = lerp(m_start, m_target, m_total);
    }

    T getCurrent() const
    {
      return m_current;
    }

    bool isDone() const
    {
      return m_total >= 1.f;
    }
  private:
    T m_start;
    T m_target;
    T m_current;

    float m_total;
    float m_timestep;
  };
}

#endif
