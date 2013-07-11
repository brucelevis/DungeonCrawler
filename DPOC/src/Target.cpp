#include "logger.h"
#include "Target.h"

std::string targetToString(Target target)
{
  switch (target)
  {
  case TARGET_SINGLE_ENEMY:
    return "TARGET_SINGLE_ENEMY";
  case TARGET_SINGLE_ALLY:
    return "TARGET_SINGLE_ALLY";
  case TARGET_ALL_ENEMY:
    return "TARGET_ALL_ENEMY";
  case TARGET_ALL_ALLY:
    return "TARGET_ALL_ALLY";
  case TARGET_SELF:
    return "TARGET_SELF";
  case TARGET_NONE:
    return "TARGET_NONE";
  }

  TRACE("Unknown target %d", (int)target);

  return "";
}

Target targetFromString(const std::string& str)
{
  if (str == "TARGET_SINGLE_ENEMY")
    return TARGET_SINGLE_ENEMY;
  if (str == "TARGET_SINGLE_ALLY")
    return TARGET_SINGLE_ALLY;
  if (str == "TARGET_ALL_ENEMY")
    return TARGET_ALL_ENEMY;
  if (str == "TARGET_ALL_ALLY")
    return TARGET_ALL_ALLY;
  if (str == "TARGET_SELF")
    return TARGET_SELF;
  if (str == "TARGET_NONE")
    return TARGET_NONE;

  TRACE("Unknown target %s, returning TARGET_NONE", str.c_str());

  return TARGET_NONE;
}
