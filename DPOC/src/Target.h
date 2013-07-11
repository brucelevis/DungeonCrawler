#ifndef TARGET_H
#define TARGET_H

#include <string>

enum Target
{
  TARGET_SINGLE_ENEMY,
  TARGET_SINGLE_ALLY,
  TARGET_ALL_ENEMY,
  TARGET_ALL_ALLY,
  TARGET_SELF,
  TARGET_NONE
};

std::string targetToString(Target target);
Target targetFromString(const std::string& str);

#endif
