#include <sstream>
#include <algorithm>
#include <ctype.h>

#include "Utility.h"

std::string get_equip_short_name(const std::string& equipName)
{
  if (equipName == "Weapon")
  {
    return "Weap";
  }
  else if (equipName == "Shield")
  {
    return "Shld";
  }
  else if (equipName == "Armour")
  {
    return "Armr";
  }
  else if (equipName == "Helmet")
  {
    return "Helm";
  }
  else if (equipName == "Others")
  {
    return "Misc";
  }

  return "";
}
