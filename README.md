Map format (Tiled)
------------------
First tileset is the map tileset. Other tilesets are used for entities.

Properties:
 * music
 * encounterRate (1 / x per step for random encounter, default 30)
 * zone:x (x = zoneId for encounter zone)
  - value -> comma separated list of monster groups, and groups for the zone
    are separated by pipes.
  - example: Batty|Batty,Centopode,Centopode|Batty,Batty

Tile layers:
 * Two currently supported
 * Layer called "blocking" used for solid tiles.

Objects:
 * Objects using tiles are "entities" in the game. Objects using tiles from
   first tileset are "tile entities".

 * Object properties:
  - walkSpeed (float)
  - walkThrough (true/false)
  - fixedDirection (true/false)

 * Warps:
  - Objects with name set to "warp" are warp objects.
  - destX, destY and destMap properties required.

 * Zones:
  - Objects with name set to "zone" represents a zone for encounters.
  - property zoneId required.

XML Formats
-----------

### Spells.xml (`<spells><spell>`) ###
* `<name>`
* `<description>`
* `<cost>` (default 0)
* `<target>` (default TARGET_NONE)
* `<battleOnly>` (default true)
* `<power>` (default 0)
* `<effect>` (effect animation)
* `<element>` (spell element)
* `<spellType>`
 - `<type>`*
* `<statusChange>`
 - `<status name, {chance, duration}>`* (chance default 100, duration default 0)
* `<buff>`
 - `<attribute name>`*

#### Spell types: ####
 * SPELL_NONE
 * SPELL_DAMAGE
 * SPELL_HEAL
 * SPELL_BUFF
 * SPELL_REMOVE_STATUS
 * SPELL_CAUSE_STATUS
 * SPELL_CUSTOM

### Items.xml (`<items><item>`) ###
* `<name>`
* `<description>`
* `<cost>`
* `<type>`
* `<target>`
* `<onUse>`
* `<status>`
* `<effect>`
* `<attributes>` (gain when use, bonus when equipped)
 - `<attribute name, value>`*
* `<elements>` (damage element, protection from element)
 - `<element name, value>`* (value = multiplier in damage calculation)

#### type: ####
 * ITEM_USE
 * ITEM_USE_MENU
 * ITEM_USE_BATTLE
 * ITEM_WEAPON
 * ITEM_SHIELD
 * ITEM_ARMOR
 * ITEM_HELMET
 * ITEM_MISC

#### onUse: ####
 * ITEM_HEAL
 * ITEM_HEAL_FIXED
 * ITEM_RESTORE_MP
 * ITEM_RESTORE_MP_FIXED
 * ITEM_DAMAGE
 * ITEM_BUFF
 * ITEM_REMOVE_STATUS
 * ITEM_CAUSE_STATUS
 * ITEM_CUSTOM

### Monsters.xml (`<monsters><monster>`) ###
* `<name>`
* `<description>`
* `<texture name, x, y, w, h>` (rect in atlas)
* `<color r, g, b>` (default white)
* `<attributes>`
 - `<attr name, value>`*
* `<actions>`
 - `<action name, chance>{SpellName}</action>`*
* `<items>` (item drops)
 - `<item name, chance>`*
* `<resistance>`
 - `<element name, resist>`* (resist is used as multiplier in dmg calculation)
* `<immunity>`
 - `<status name>`*

### Config.xml (`<config>`) ###
Just a key-value store.

Example:  
 `<KEY>Value</KEY>`  
 Value can be retrieved then with config::get("KEY")

### Classes.xml (`<classes><class>`) ###
* `<name>`
* `<attributes>`  (BASE attributes used when leveling.)
 - `<attr name, value>`*
* `<texture name, x, y>` (entry in texture 2x4 texture block)
* `<face name x, y, w, h>` (w, h should be 32x32)
* `<spellsPerLevel>`
 - `<level num>`*
   + `<spell>SpellName</spell>`*
* `<equipment>` (equipment this class can use)
 - `<item>ItemName</item>`*

### Player.xml (`<player>`) ###
Initial values for the player.

* `<start map, x, y>`
* `<party>`
 - `<character name, class>`*
* `<inventory>`
 - `<item name, amount>`*

### StatusEffects.xml (`<statusEffects><statusEffect>`) ###
* `<name>`
* `<verb>` (Text to print when effect is activated)
* `<recoveryVerb>` (Text to print when recovery)
* `<color r, g, b>` (Color to use for status)
* `<battleOnly>` (Default true)
* `<recoveryChance>` (% chance of recovery every turn, default 0)
* `<incapacitate>` (default false. Character not usable if true)
* `<damage type, amount, attr>`
* `<sound>` (Sound to play when effect activates)
* `<statusType>`
 - `<type>StatusType</type>`*

####Damage types:####
* DAMAGE_NONE - No damage default
* DAMAGE_FIXED - Fixed damge per turn
* DAMAGE_PERCENT - Damage = % of max hp/mp

####Status effect types:####
These are hardcoded behaviors.
* STATUS_NONE
* STATUS_CONFUSE
 - Chance to attack random target instead of usual action
 - Chance to select random target with usual action.
 - Chance to fumble
* STATUS_FUMBLE  
 - 50% chance to lose turn.
* STATUS_BLIND   
 - 50% miss chance.
* STATUS_REFLECT 
 - Single target spells rebound to caster.
* STATUS_PROVOKE 
 - When targets are randomly selected provoking actor is always picked.
* STATUS_SILENCE 
 - Can't cast spells.

Attributes
----------
* hp
* strength
* power
* defense
* magic
* mag.def
* speed
* level
* exp   (exp gain by monsters)
* gold  (gold gain by monsters)

Script Commands
---------------
* message [msg]
 - Displays a message box with msg.
* walk [dir]
 - Entity takes a step in dir, one of DIR_RIGHT, DIR_LEFT, DIR_DOWN, DIR_UP,
 - DIR_RANDOM
* set_dir [dir]
 - Entity changes direction to dir. Overrides fixed direction.
* wait [frames]
 - Entity waits for some frames before continuing script
* set_global [key] [value]
 - Set global variable.
* set_local [key] [value]
 - Set local variable. This depends on the name of the entity so if several
   entities have the same name on the same map then they will share locals.
* if [condition]  
   ...  
  {else}  
   ...  
  endif  
  Bool operations: `<, >, <=, >`=, ==, !=  
  Valid things: local[key],  
                global[key],  
                const[number|true|false],  
                item[item_name] (use _ for spaces here; item_name=gold for gold
* choice [choice0,choice1,...,choiceN]
 - Pops up a choice menu. The selected choice is stored in global[sys:choice].
   Hardcoded limit to 4 choices right now. Comma separated list.
* set_tile_id [tilenum]
 - If the entity is a tile entity this changes the tile number.
* give_item [amount] [item_name]
* take_item [amount] [item_name]
* give_gold [amount]
* take_gold [amount]
* play_sound [sound]
* add_member [name] [class] {level}
 - Adds a new member to the player party.
* remove_member [name]
* set_visible [true/false]
 - Sets the visibilty of the entity
* set_walkthrough [true/false]
 - If walkthrough is false (default) the entity is solid.
* enable_controls [true/false]
 - Enable/disable player controls
* recover_all
 - Restore all hp/mp/status to player
* combat [monster1,monster2,...,monsterN]
 - Comma separated list of monsters to fight.  
   ex: combat Batty,Monster With Space In Name,Skelington
* end_game
* set_config [key] [value]
* transfer [targetMap] [x] [y]

Formulas
--------
* Miss chance:
 - if target is faster, miss chance is rand(255) < (targetSpeed - attackerSpeed)
 - else 1/255 chance