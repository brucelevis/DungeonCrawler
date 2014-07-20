#ifndef TRAP_H
#define TRAP_H

#include <string>
#include <vector>

class PlayerCharacter;

struct Trap
{
  int x, y;

  Trap(const std::string& type, int luck, int _x, int _y);

  void checkTrap() const;
private:
  void applyTrap(const std::vector<PlayerCharacter*>& party) const;
private:
  std::string m_type;
  int m_luckToBeat;
};

#endif
