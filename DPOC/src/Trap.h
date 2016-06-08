#ifndef TRAP_H
#define TRAP_H

#include <string>
#include <vector>

class PlayerCharacter;

struct Trap
{
  int x, y;

  Trap();
  Trap(const std::string& type, int difficulty, int _x, int _y);

  PlayerCharacter* triggerTrap() const;
  void checkTrap() const;
  void applyTrap(const std::vector<PlayerCharacter*>& party) const;
private:
  std::string m_type;
  int m_difficulty;
};

#endif
