#include <vector>

#include "logger.h"
#include "Monster.h"

static std::vector<MonsterDef> monsters =
{
  {
    BUG_MONSTER
  },

  {
    "Skelington", "A spooky skeleton",
    {
      { "hp",       5  },
      { "mp",       0   },
      { "strength", 14  },
      { "power",    18  },
      { "defense",  10  },
      { "magic",    8   },
      { "mag.def",  8   },
      { "speed",    12  },
      { "level",    1   },
      { "exp",      8   },
      { "gold",     4   }
    },
    "Resources/rpg_monster_set.png",
    { 12, 32, 32, 36 },

    {
      { "Attack", "", 3 },
      { "Spell", "Hurt All", 3 },
      { "Spell", "Heal", 3 }
    }
  }
};

MonsterDef get_monster_definition(const std::string& name)
{
  for (auto it = monsters.begin(); it != monsters.end(); ++it)
  {
    if (it->name == name)
      return *it;
  }

  TRACE("No monster with name %s defined!", name.c_str());

  return monsters[0];
}

std::string get_monster_description(const std::string& name)
{
  return get_monster_definition(name).description;
}
