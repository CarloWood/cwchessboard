#pragma once

#include "LinuxChessIconFactory.h"
#include "debug.h"
#include <gtkmm.h>
#include <array>
#include <map>
#include <any>

class LinuxChessWindow;

namespace menu_keys {

  enum TopEntries
  {
    File,
    Game,
    Mode,
    number_of_top_entries
  };

} // namespace menu_keys

class LinuxChessMenuBar : public Gtk::MenuBar, public LinuxChessIconFactory
{
  using TopEntries = menu_keys::TopEntries;
  using MenuEntryWithIconId = menu_keys::MenuEntryWithIconId;
  using MenuEntryWithoutIconId = menu_keys::MenuEntryWithoutIconId;

 public:
  LinuxChessMenuBar(LinuxChessWindow* main_window);

  struct MenuEntryKey
  {
    TopEntries m_top_entry;
    enum Type { is_stock_id_n, is_menu_entry_without_icon_id_n } m_type;
    Gtk::StockID m_stock_id;                                            // Valid iff m_type == is_stock_id_n.
    MenuEntryWithoutIconId m_menu_entry_without_icon_id;                // Valid iff m_type == is_menu_entry_without_icon_id_n.

    MenuEntryKey(TopEntries top_entry, Gtk::StockID stock_id)
      : m_top_entry(top_entry), m_type(is_stock_id_n), m_stock_id(stock_id) { }

    MenuEntryKey(TopEntries top_entry, MenuEntryWithoutIconId menu_entry_without_icon_id)
      : m_top_entry(top_entry), m_type(is_menu_entry_without_icon_id_n), m_menu_entry_without_icon_id(menu_entry_without_icon_id) { }

    bool is_stock_id() const
    {
      return m_type == is_stock_id_n;
    }

    bool is_menu_entry_without_icon_id() const
    {
      return m_type == is_menu_entry_without_icon_id_n;
    }

    Gtk::StockID get_stock_id() const { return m_stock_id; }
    MenuEntryWithoutIconId get_menu_entry_without_icon_id() const { return m_menu_entry_without_icon_id; }

    friend bool operator<(MenuEntryKey const& key1, MenuEntryKey const& key2)
    {
      return key1.m_top_entry < key2.m_top_entry ||
        (key1.m_top_entry == key2.m_top_entry &&
         ((key1.is_stock_id() && key2.is_menu_entry_without_icon_id()) ||
         ((key1.is_stock_id() == key2.is_stock_id() &&
          ((key1.is_stock_id() &&
            key1.get_stock_id() < key2.get_stock_id()) ||
           (key1.is_menu_entry_without_icon_id() &&
            key1.get_menu_entry_without_icon_id() < key2.get_menu_entry_without_icon_id()))))));
    }

#ifdef CWDEBUG
    friend std::ostream& operator<<(std::ostream& os, MenuEntryKey const& menu_entry_key)
    {
      os << "{top_entry:" << menu_entry_key.m_top_entry << ", menu_entry:";
      if (menu_entry_key.is_stock_id())
        os << menu_entry_key.get_stock_id();
      else if (menu_entry_key.is_menu_entry_without_icon_id())
        os << menu_keys::get_label(menu_entry_key.get_menu_entry_without_icon_id());
      else
        os << "CORRUPT MenuEntryKey";
      return os << "}";
    }
#endif
  };

 protected:
  std::array<Gtk::Menu*, menu_keys::number_of_top_entries> m_submenus;
  std::map<MenuEntryKey, Gtk::MenuItem*> m_menu_items;

  Gtk::SeparatorMenuItem* m_separator;

 public:
  template<class T>
  void append_menu_entry(MenuEntryKey menu_entry_key, T* obj, void (T::*cb)())
  {
    Gtk::MenuItem* menu_item_ptr;
    if (menu_entry_key.is_stock_id())
      menu_item_ptr = Gtk::manage(new Gtk::ImageMenuItem(menu_entry_key.get_stock_id()));
    else if (menu_entry_key.is_menu_entry_without_icon_id())
      menu_item_ptr = Gtk::manage(new Gtk::MenuItem(get_label(menu_entry_key.get_menu_entry_without_icon_id())));
    else
      ASSERT(false);
    m_menu_items[menu_entry_key] = menu_item_ptr;
    m_submenus[menu_entry_key.m_top_entry]->append(*menu_item_ptr);
    menu_item_ptr->signal_activate().connect(sigc::mem_fun(*obj, cb));
  }

  void append_separator(int top_entry)
  {
    Gtk::MenuItem* separator;
    separator = Gtk::manage(new Gtk::SeparatorMenuItem);
    m_submenus[top_entry]->append(*separator);
  }

  struct MenuEntryKeyStub
  {
    TopEntries m_top_entry;
    MenuEntryWithIconId m_menu_entry;
  };

  template<class T>
  void append_menu_entry(MenuEntryKeyStub menu_entry_key, T* obj, void (T::*cb)())
  {
    append_menu_entry({menu_entry_key.m_top_entry, get_icon_id(menu_entry_key.m_menu_entry)}, obj, cb);
  }

  template<class T>
  void append_radio_menu_entry(Gtk::RadioButtonGroup& group, MenuEntryKey menu_entry_key, T* obj, void (T::*cb)())
  {
    ASSERT(menu_entry_key.is_menu_entry_without_icon_id());     // None of our radio menu items have icons.
    Gtk::RadioMenuItem* menu_item_ptr = Gtk::manage(new Gtk::RadioMenuItem(group, get_label(menu_entry_key.get_menu_entry_without_icon_id())));
    m_menu_items[menu_entry_key] = menu_item_ptr;
    m_submenus[menu_entry_key.m_top_entry]->append(*menu_item_ptr);
    menu_item_ptr->signal_activate().connect(sigc::mem_fun(*obj, cb));
  }

  void activate(MenuEntryKey menu_entry_key)
  {
    auto item = m_menu_items.find(menu_entry_key);
    if (item != m_menu_items.end())
      item->second->activate();
  }

  void set_sensitive(bool sensitive, MenuEntryKey menu_entry_key)
  {
    auto item = m_menu_items.find(menu_entry_key);
    if (item != m_menu_items.end())
      item->second->set_sensitive(sensitive);
  }

 private:
  static char const* top_entry_label(TopEntries top_entry);
};
