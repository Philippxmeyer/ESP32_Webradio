#ifndef WEBRADIO_STATIONMENU_H
#define WEBRADIO_STATIONMENU_H
#include "StationStore.h"
// Entries carry vector positions; current group and parents use persistent IDs.
class StationMenu {
public:
  enum class Kind { Group, Station, Back };
  enum class Action { Navigate, Play, Close };
  struct Entry { Kind kind; size_t index; };
  void reset() { group_ = 0; selected_ = 0; }
  uint32_t group() const { return group_; }
  size_t selected() const { return selected_; }
  void select(size_t index) { selected_ = index < count() ? index : count() - 1; }
  const char* title() const { return stationStore.group(group_).name.c_str(); }
  size_t count() const {
    size_t total = 1;
    for (size_t i = 1; i < stationStore.groupCount(); ++i)
      if (stationStore.groupAt(i).parent == group_) ++total;
    for (size_t i = 0; i < stationStore.count(); ++i)
      if (stationStore.at(i).group == group_) ++total;
    return total;
  }
  Entry entry(size_t position) const {
    for (size_t i = 1; i < stationStore.groupCount(); ++i)
      if (stationStore.groupAt(i).parent == group_) {
        if (position == 0) return {Kind::Group, i};
        --position;
      }
    for (size_t i = 0; i < stationStore.count(); ++i)
      if (stationStore.at(i).group == group_) {
        if (position == 0) return {Kind::Station, i};
        --position;
      }
    return {Kind::Back, 0};
  }
  const char* name(size_t position) const {
    const Entry item = entry(position);
    if (item.kind == Kind::Group) return stationStore.groupAt(item.index).name.c_str();
    if (item.kind == Kind::Station) return stationStore.at(item.index).name.c_str();
    return "zurück";
  }
  Action activate(size_t& station) {
    const Entry item = entry(selected_);
    if (item.kind == Kind::Station) { station = item.index; return Action::Play; }
    if (item.kind == Kind::Group) {
      group_ = stationStore.groupAt(item.index).id;
      selected_ = 0;
    } else {
      if (group_ == 0) return Action::Close;
      const size_t child = stationStore.findGroup(group_);
      group_ = stationStore.group(group_).parent;
      selected_ = 0;
      for (size_t i = 1; i < child; ++i)
        if (stationStore.groupAt(i).parent == group_) ++selected_;
    }
    return Action::Navigate;
  }
private:
  uint32_t group_ = 0;
  size_t selected_ = 0;
};
#endif
