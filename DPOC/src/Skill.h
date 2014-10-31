#ifndef SKILL_H_
#define SKILL_H_

#include <string>

struct Skill
{
  std::string name;
  int ranks;
  int costOfRank;

  int getRanks(int percent) const;
  int getPercent(int rank) const;

  static const Skill& get(const std::string& name);
  static std::vector<std::string> getAllSkills();
  static bool isSkill(const std::string& name);
};

void load_skills();

#endif /* SKILL_H_ */
