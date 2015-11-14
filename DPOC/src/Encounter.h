#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include <string>
#include <vector>

struct Encounter
{
  Encounter();

  std::string name;
  std::string music;
  std::vector<std::string> monsters;
  std::vector<std::string> script;
  bool canEscape;

  void start() const;
};

void load_encounters();
const Encounter* get_encounter(const std::string& encounterName);

#endif
