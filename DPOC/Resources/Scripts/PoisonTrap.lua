local party_size = get_party_size()

for i = 1, party_size, 1 do
  local member = get_party_member(i - 1)

  if not check_vs_luck(get_attribute(member, "luck"), 9999) then
    afflict_status(member, "Poison", -1)

    message(get_character_name(member) .. " was poisoned!")
  end
end
