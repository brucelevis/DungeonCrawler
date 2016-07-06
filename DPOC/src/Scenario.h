#ifndef DPOC_SRC_SCENARIO_H_
#define DPOC_SRC_SCENARIO_H_

#include <string>

/**
 * Configuration and options related to the current scenario.
 */
class Scenario
{
public:
  static Scenario& instance();

  bool useCharGen() const { return m_useCharGen; }
  std::string getName() const { return m_name; }
private:
  Scenario();
private:
  bool m_useCharGen;
  std::string m_name;
};

#endif /* DPOC_SRC_SCENARIO_H_ */
