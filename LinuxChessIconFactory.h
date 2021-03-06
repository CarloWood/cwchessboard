#pragma once

#include <gtkmm.h>
#include <array>

namespace menu_keys {

enum MenuEntryWithIconId
{
  Export,
  Flip,
  number_of_custom_icons
};

enum MenuEntryWithoutIconId
{
  ShowCandidates,
  ShowReachables,
  ShowAttacked,
  ShowDefendables,
  ShowDefendedBlack,
  ShowDefendedWhite,
  ShowMoves,
  PlacePieces,
  EditPosition,
  EditGame
};

std::string get_label(MenuEntryWithoutIconId menu_entry_id);

} // namespace menu_keys

class LinuxChessIconFactory
{
 public:
  LinuxChessIconFactory();

  Gtk::StockID get_icon_id(menu_keys::MenuEntryWithIconId menu_entry_id) const { return m_icon_ids[menu_entry_id]; }

 private:
  struct icon_info_st
  {
    char const* filepath;
    char const* id;
    char const* label;
  };

 protected:
  static std::array<icon_info_st, menu_keys::number_of_custom_icons> s_icon_info;
  std::array<Gtk::StockID, menu_keys::number_of_custom_icons> m_icon_ids;
};
