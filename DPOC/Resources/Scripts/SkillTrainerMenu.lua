currentCharacter = nil
height = 0
skills = nil

function constructor(menu)
end

function destructor(menu)
end

function handle_confirm(menu)
  local current_skill = get_current_skill(menu)
  local skill_value = get_base_attribute(currentCharacter, current_skill)
  local skill_points = get_base_attribute(currentCharacter, Term_SkillPoints)
  local skill = get_skill(current_skill)
  local skill_cost = get_skill_cost_of_rank(skill)

  if skill_points >= skill_cost and skill_value < 100 then
    play_sound(get_config_var("SOUND_SHOP"))

    advance_attribute(currentCharacter, current_skill, get_skill_percent(1))
    advance_attribute(currentCharacter, Term_SkillPoints, -skill_cost)

    update_entries(menu)
  else
    play_sound(get_config_var("SOUND_CANCEL"))
  end
end

function handle_escape(menu)
  _super_handle_escape(menu)
end

function move_arrow(menu, dir)
  _super_move_arrow(menu, dir)
end

function draw(menu, target, x, y)
  draw_frame(target, 0, 0, GAME_RES_X, 24)

  local skill = get_skill(get_current_skill(menu))
  local skill_value = get_base_attribute(currentCharacter, get_current_skill(menu))
  local skill_cost = get_skill_cost_of_rank(skill)
  local skill_points = get_base_attribute(currentCharacter, Term_SkillPoints)

  if (skill_value < 100) then
    draw_text(target, vocab_short(Term_SkillPoints) .. " cost: " .. skill_cost .. " / " .. skill_points)
  else
    draw_text(target, "Skill maxed out!", 8, 8)
  end

  _super_draw(menu, target, x, y + 22)
end

function get_width(menu)
  return GAME_RES_X
end

function get_height(menu)
  return height
end

function invalidate(menu)
  _protected_set_max_visible(height / 12)
  if currentCharacter ~= nil then
    update_entries(menu)
  end
end

function get_current_skill(menu)
  local choice = get_current_menu_choice(menu)
  local choice_vec = split_string(choice, " ")

  if vector_string_size(choice_vec) > 0 then
    return vector_string_at(choice_vec, 0)
  end

  return ""
end

function update_entries(menu)
  menu_clear(menu)

  for i = 0, vector_string_size(skills) - 1 do
    local skill = vector_string_at(skills, i)

    -- Add char skill % to entry as well as space padding to line stuff up
    -- nicely.
    local percent = tostring(get_base_attribute(currentCharacter, skill)) .. "%"
    local length = strlen(skill) + strlen(percent)
    local space_pad = get_width(menu) / 8 - length - 3
    local entry = skill

    for j = 0, spade_pad - 1 do
      entry = entry .. " "
    end
    entry = entry .. percent

    menu_add_entry(menu, entry)
  end
end
