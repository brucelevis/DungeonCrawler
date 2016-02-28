STATE_STARTING = 0
STATE_START_MESSAGE = 1
STATE_MESSAGE_ONGOING = 2
STATE_CHOOSE = 3
STATE_LEAVE = 4
STATE_REST = 5

state = 0
required_food = 0
do_draw = true
background = nil
menu = nil
music = nil
current_message = ""

function constructor(scene)
  state = STATE_STARTING

  background = load_texture("Backgrounds/Campsite.png")

  menu = create_choice_menu()
  menu_add_entry(menu, "Rest")
  menu_add_entry(menu, "Leave")
  menu_set_visible(menu, false)

  music = load_music("Music/Inn.ogg")

  required_food = compute_required_food()

  fade_in(32)
end

function destructor(scene)
  free_texture(background)
  free_music(music)
  free_menu(menu)
end

function update(scene)
  if state == STATE_START_MESSAGE then
    show_message(current_message)
    state = STATE_MESSAGE_ONGOING
  elseif state == STATE_MESSAGE_ONGOING then
    update_message()

    if message_waiting_for_key() then
      state = STATE_CHOOSE
      menu_set_visible(menu, true)
    end
  elseif state == STATE_REST then
    if not music_is_playing(music) then
      leave()
    end
  end
end

function draw(scene, target)
  if do_draw then
    draw_texture(background, target, 0, 0)

    if state ~= STATE_REST and state ~= STATE_STARTING and state ~= STATE_LEAVE then
      draw_message_box(target)
    end

    if menu_is_visible then
      draw_menu(target, menu, 0, GAME_RES_Y - 68 - get_menu_height(menu))
    end

    draw_frame(target, 0, 0, 128, 24)
    draw_text(target, "Food: " .. required_food .. " / " .. get_global("$sys:food"), 8, 8)
  end
end

function handle_event(scene, event)
  if event_type(event) == KeyPressed then
    local key = get_keycode(event)

    if key == Key_Space then
      if state == STATE_MESSAGE_ONGOING then
        if not message_waiting_for_key() then
          flush_message()
        end
      elseif state == STATE_CHOOSE then
        if get_current_menu_choice(menu) == "Rest" then
          if get_global("$sys:food") >= required_food then
            rest()
            set_global("$sys:food", get_global("$sys:food") - required_food)
          else
            start_message("You don't have enough food to rest right now.")
            menu_set_visible(false)
            play_sound(get_config_var("SOUND_CANCEL"))
          end
        elseif get_current_menu_choice(menu) == "Leave" then
          leave()
        end
      end
    elseif key == Key_Up and state == STATE_CHOOSE then
      move_menu_arrow(menu, DIR_UP)
    elseif key == Key_Down and state == STATE_CHOOSE then
      move_menu_arrow(menu, DIR_DOWN)
    elseif key == Key_Escape and state == STATE_CHOOSE then
      leave()
    end
  end
end

function pre_fade(scene, fade_type)
end

function post_fade(scene, fade_type)
  if fade_type == FADE_OUT then
    if state == STATE_LEAVE then
      fade_in(32)
      close(scene)
    elseif state == STATE_REST then
      do_draw = false
    end
  elseif fade_type == FADE_IN then
    if state == STATE_STARTING then
      start_message("You feel the warm glow of the camp site surround you. It is safe to rest here.");
    end
  end
end

function leave()
  clear_message()
  state = STATE_LEAVE
  fade_out(32)
end

function rest()
  recover_party(get_player())

  state = STATE_REST
  play_music(music)
  fade_out(32)
end

function compute_required_food()
  local sum_level = 0
  local multiplier = 0

  for i = 0, get_party_size() - 1 do
    local character = get_party_member(i)

    sum_level = sum_level + get_attribute(character, "level")

    if character_has_status(character, "Dead") then
      multiplier = multiplier + 0.5
    else
      local max_hp = get_max_attribute(character, "hp")
      local cur_hp = get_current_attribute(character, "hp")

      local quote = cur_hp / max_hp

      if quote < 0.25 then
        multiplier = multiplier + 0.15
      elseif quote < 0.5 then
        multiplier = multiplier + 0.1
      elseif quote < 0.75 then
        multiplier = multiplier + 0.05
      end
    end
  end

  return 0 * sum_level * multiplier
end

function start_message(message)
  clear_message()
  current_message = message
  state = STATE_START_MESSAGE
end
