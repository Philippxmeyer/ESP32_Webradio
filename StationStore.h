#ifndef WEBRADIO_STATIONSTORE_H
#define WEBRADIO_STATIONSTORE_H
#include <Preferences.h>
#include <vector>
#include "Stations.h"
struct SavedStation {
  uint32_t id;
  String name, url;
  uint32_t group;
};
struct SavedGroup {
  uint32_t id;
  String name;
  uint32_t parent;
};
// One validated configuration is committed to NVS, then published to the loop.
// The audio task only receives copied station IDs and URLs.
class StationStore {
public:
  static constexpr size_t MAX_STATIONS = 48, MAX_GROUPS = 24, MAX_NAME_BYTES = 63;
  static constexpr size_t MAX_URL_BYTES = 255, MAX_JSON_BYTES = 8192, MAX_HOSTNAME = 31;
  void begin();
  size_t count() const { return state_.stations.size(); }
  const SavedStation& at(size_t index) const { return state_.stations[index]; }
  size_t groupCount() const { return state_.groups.size(); }
  const SavedGroup& groupAt(size_t index) const { return state_.groups[index]; }
  const SavedGroup& group(uint32_t id) const;
  size_t find(uint32_t id) const;
  size_t findGroup(uint32_t id) const;
  const String& hostname() const { return state_.hostname; }
  String json() const;
  bool replace(const String& json, String& error);
private:
  struct State {
    std::vector<SavedStation> stations;
    std::vector<SavedGroup> groups;
    uint32_t nextId = STATION_COUNT + 1, nextGroupId = STATION_GROUP_COUNT;
    String hostname = "webradio";
  };
  bool decode(const String& json, bool loading, State& result, String& error) const;
  static String encode(const State& state);
  Preferences preferences_;
  bool ready_ = false;
  State state_;
};
extern StationStore stationStore;
#endif
