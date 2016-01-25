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
}

#endif /* VOCABULARY_H_ */
