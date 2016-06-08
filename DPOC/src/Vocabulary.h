#ifndef VOCABULARY_H_
#define VOCABULARY_H_

#include <string>

void load_vocabulary();

const std::string& vocab(const std::string& termName);
std::string vocab_upcase(const std::string& termName);
const std::string& vocab_mid(const std::string& termName);
const std::string& vocab_short(const std::string& termName);

namespace terms
{
  // Attribute terms
  const std::string gold = "gold";

  const std::string hp = "hp";
  const std::string mp = "mp";

  const std::string strength = "strength";
  const std::string defense = "defense";
  const std::string magic = "magic";
  const std::string magdef = "mag.def";
  const std::string speed = "speed";
  const std::string luck = "luck";

  const std::string exp = "exp";
  const std::string level = "level";

  const std::string skillpoints = "skillpoints";

  // Equipment terms
  const std::string weapon = "Weapon";
  const std::string shield = "Shield";
  const std::string armour = "Armour";
  const std::string helmet = "Helmet";
  const std::string misc1  = "Misc1";
  const std::string misc2  = "Misc2";

  // Skill names
  const std::string searching = "Searching";
  const std::string mechanics = "Mechanics";
  const std::string evasion   = "Evasion";
  const std::string merchant  = "Merchant";
}

#endif /* VOCABULARY_H_ */
