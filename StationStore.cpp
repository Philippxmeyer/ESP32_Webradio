#include "StationStore.h"
#include <cJSON.h>
#include <cmath>
#include <memory>
#include <limits>
StationStore stationStore;
namespace {
using Json = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>;
bool integer(const cJSON* value, double min, double max) {
  return cJSON_IsNumber(value) && std::isfinite(value->valuedouble) &&
    value->valuedouble >= min && value->valuedouble <= max &&
    std::floor(value->valuedouble) == value->valuedouble;
}
// Bound parser recursion before cJSON sees untrusted input.
bool shallowJson(const String& text) {
  int depth = 0;
  bool quoted = false, escaped = false;
  for (size_t i = 0; i < text.length(); ++i) {
    const char c = text[i];
    if (c == 0) return false;
    if (escaped) { escaped = false; continue; }
    if (quoted && c == '\\') {
      if (text.substring(i + 1, i + 6) == "u0000") return false;
      escaped = true; continue;
    }
    if (c == '"') quoted = !quoted;
    if (!quoted && (c == '{' || c == '[') && ++depth > 6) return false;
    if (!quoted && (c == '}' || c == ']') && --depth < 0) return false;
  }
  return !quoted && depth == 0;
}
bool textValid(const String& text) {
  // Well-formed UTF-8, no control characters, no overlong encodings/surrogates.
  const auto* s = reinterpret_cast<const unsigned char*>(text.c_str());
  for (size_t i = 0; i < text.length();) {
    uint32_t cp = s[i++];
    if (cp < 0x20 || cp == 0x7f) return false;
    if (cp < 0x80) continue;
    unsigned more;
    uint32_t minimum;
    if (cp >= 0xc2 && cp <= 0xdf) { more = 1; minimum = 0x80; cp &= 0x1f; }
    else if (cp >= 0xe0 && cp <= 0xef) { more = 2; minimum = 0x800; cp &= 0x0f; }
    else if (cp >= 0xf0 && cp <= 0xf4) { more = 3; minimum = 0x10000; cp &= 7; }
    else return false;
    if (i + more > text.length()) return false;
    while (more--) {
      if ((s[i] & 0xc0) != 0x80) return false;
      cp = (cp << 6) | (s[i++] & 0x3f);
    }
    if (cp < minimum || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff) ||
        (cp >= 0x80 && cp <= 0x9f)) return false;
  }
  return true;
}
bool urlValid(const String& url) {
  const size_t start = url.startsWith("https://") ? 8 : url.startsWith("http://") ? 7 : 0;
  if (!start || url.length() <= start || url[start] == '/' || url[start] == '?' || url[start] == '#') return false;
  for (size_t i = 0; i < url.length(); ++i)
    if (uint8_t(url[i]) <= 0x20 || uint8_t(url[i]) >= 0x7f || url[i] == '\\') return false;
  return true;
}
}
size_t StationStore::find(uint32_t id) const {
  for (size_t i = 0; i < count(); ++i) if (state_.stations[i].id == id) return i;
  return count();
}
size_t StationStore::findGroup(uint32_t id) const {
  for (size_t i = 0; i < groupCount(); ++i) if (state_.groups[i].id == id) return i;
  return groupCount();
}
const SavedGroup& StationStore::group(uint32_t id) const {
  const size_t index = findGroup(id);
  return state_.groups[index < groupCount() ? index : 0];
}
void StationStore::begin() {
  state_ = State{};
  for (size_t i = 0; i < STATION_GROUP_COUNT; ++i)
    state_.groups.push_back({uint32_t(i), i ? STATION_GROUPS[i].name : "Startseite", uint32_t(STATION_GROUPS[i].parent)});
  for (size_t i = 0; i < STATION_COUNT; ++i)
    state_.stations.push_back({uint32_t(i + 1), STATIONS[i].name, STATIONS[i].url, uint32_t(STATIONS[i].group)});
  ready_ = preferences_.begin("radio-list", false);
  if (!ready_) { Serial.println("Radiokonfiguration: NVS nicht verfuegbar"); return; }
  const size_t length = preferences_.getBytesLength("list");
  if (!length) return;
  if (length > MAX_JSON_BYTES) { Serial.println("Radiokonfiguration: gespeicherte Daten zu gross"); return; }
  std::unique_ptr<char[]> buffer(new(std::nothrow) char[length + 1]);
  if (!buffer || preferences_.getBytes("list", buffer.get(), length) != length) return;
  buffer[length] = 0;
  String error;
  State loaded;
  if (decode(String(buffer.get()), true, loaded, error)) state_ = std::move(loaded);
  else Serial.println("Radiokonfiguration: Standardwerte verwendet: " + error);
}
bool StationStore::decode(const String& text, bool loading, State& result, String& error) const {
  if (text.isEmpty() || text.length() > MAX_JSON_BYTES || !shallowJson(text)) {
    error = "Ungültige oder zu große Radiokonfiguration."; return false;
  }
  Json root(cJSON_ParseWithLengthOpts(text.c_str(), text.length() + 1, nullptr, true), cJSON_Delete);
  if (!cJSON_IsObject(root.get())) { error = "Ungültige Radiokonfiguration."; return false; }
  result.nextId = state_.nextId;
  result.nextGroupId = state_.nextGroupId;
  result.hostname = state_.hostname;
  // Legacy lists have no hostname/groups. Preserve their defaults (or current values for an old API client).
  const cJSON* hostname = cJSON_GetObjectItemCaseSensitive(root.get(), "hostname");
  if (hostname) {
    if (!cJSON_IsString(hostname)) { error = "Ungültiger Hostname."; return false; }
    result.hostname = hostname->valuestring;
    result.hostname.trim(); result.hostname.toLowerCase();
    if (result.hostname.isEmpty() || result.hostname.length() > MAX_HOSTNAME ||
        result.hostname[0] == '-' || result.hostname[result.hostname.length() - 1] == '-') {
      error = "Hostname: 1 bis 31 Buchstaben/Ziffern, Bindestriche nur innerhalb des Namens."; return false;
    }
    for (size_t i = 0; i < result.hostname.length(); ++i) {
      const char c = result.hostname[i];
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
        error = "Hostname: nur a–z, 0–9 und Bindestriche, ohne .local oder Leerzeichen."; return false;
      }
    }
  }
  const cJSON* groups = cJSON_GetObjectItemCaseSensitive(root.get(), "groups");
  if (!groups) result.groups = state_.groups;
  else {
    const int count = cJSON_GetArraySize(groups);
    if (!cJSON_IsArray(groups) || count < 1 || count > int(MAX_GROUPS)) {
      error = "Es sind 1 bis 24 Gruppen einschließlich Startseite möglich."; return false;
    }
    if (loading) {
      const cJSON* next = cJSON_GetObjectItemCaseSensitive(root.get(), "nextGroupId");
      if (!integer(next, 1, UINT32_MAX - 1)) { error = "Ungültiger Gruppen-ID-Zähler."; return false; }
      result.nextGroupId = uint32_t(next->valuedouble);
    }
    result.groups.reserve(count);
    for (int i = 0; i < count; ++i) {
      const cJSON* row = cJSON_GetArrayItem(groups, i);
      const cJSON* id = cJSON_GetObjectItemCaseSensitive(row, "id");
      const cJSON* name = cJSON_GetObjectItemCaseSensitive(row, "name");
      const cJSON* parent = cJSON_GetObjectItemCaseSensitive(row, "parent");
      if (!cJSON_IsObject(row) || !integer(id, 0, UINT32_MAX - 2) ||
          !cJSON_IsString(name) || !integer(parent, 0, UINT32_MAX - 2)) {
        error = "Eine Gruppe enthält ungültige Felder."; return false;
      }
      SavedGroup group{uint32_t(id->valuedouble), name->valuestring, uint32_t(parent->valuedouble)};
      group.name.trim();
      if (group.name.isEmpty() || group.name.length() > MAX_NAME_BYTES || !textValid(group.name)) {
        error = "Gruppennamen dürfen nicht leer sein und maximal 63 UTF-8-Bytes enthalten."; return false;
      }
      if ((i == 0 && (group.id != 0 || group.parent != 0)) || (i > 0 && group.id == 0)) {
        error = "Die Startseite muss als erste Gruppe mit ID und Eltern-ID 0 erhalten bleiben."; return false;
      }
      if ((loading && group.id >= result.nextGroupId) ||
          (!loading && group.id < state_.nextGroupId && findGroup(group.id) == groupCount())) {
        error = "Veraltete Gruppen-ID. Bitte neu laden."; return false;
      }
      if (!loading && group.id >= result.nextGroupId) result.nextGroupId = group.id + 1;
      for (const auto& previous : result.groups) if (previous.id == group.id) {
        error = "Doppelte Gruppen-ID."; return false;
      }
      result.groups.push_back(std::move(group));
    }
  }
  const auto groupIndex = [&result](uint32_t id) {
    for (size_t i = 0; i < result.groups.size(); ++i) if (result.groups[i].id == id) return i;
    return result.groups.size();
  };
  // Every group must reach the root; reject orphans, self-links and longer cycles.
  for (const auto& group : result.groups) {
    uint32_t current = group.id;
    size_t steps = 0;
    while (current) {
      const size_t index = groupIndex(current);
      if (index == result.groups.size() || ++steps > result.groups.size()) {
        error = "Gruppen dürfen keine fehlenden Eltern oder kreisförmigen Zuordnungen enthalten."; return false;
      }
      current = result.groups[index].parent;
    }
  }
  const cJSON* list = cJSON_GetObjectItemCaseSensitive(root.get(), "stations");
  const int count = cJSON_GetArraySize(list);
  if (!cJSON_IsArray(list) || count < 1 || count > int(MAX_STATIONS)) {
    error = "Die Liste muss 1 bis 48 Sender enthalten."; return false;
  }
  if (loading) {
    const cJSON* next = cJSON_GetObjectItemCaseSensitive(root.get(), "nextId");
    if (!integer(next, 1, UINT32_MAX - 1)) { error = "Ungültiger Sender-ID-Zähler."; return false; }
    result.nextId = uint32_t(next->valuedouble);
  }
  result.stations.reserve(count);
  for (int i = 0; i < count; ++i) {
    const cJSON* row = cJSON_GetArrayItem(list, i);
    const cJSON* id = cJSON_GetObjectItemCaseSensitive(row, "id");
    const cJSON* name = cJSON_GetObjectItemCaseSensitive(row, "name");
    const cJSON* url = cJSON_GetObjectItemCaseSensitive(row, "url");
    const cJSON* group = cJSON_GetObjectItemCaseSensitive(row, "group");
    if (!cJSON_IsObject(row) || !integer(id, loading ? 1 : 0, UINT32_MAX - 1) ||
        !cJSON_IsString(name) || !cJSON_IsString(url) || !integer(group, 0, UINT32_MAX - 2)) {
      error = "Sender " + String(i + 1) + ": ungültige Felder."; return false;
    }
    SavedStation station{uint32_t(id->valuedouble), name->valuestring, url->valuestring, uint32_t(group->valuedouble)};
    station.name.trim(); station.url.trim();
    if (station.name.isEmpty() || station.name.length() > MAX_NAME_BYTES || !textValid(station.name) ||
        station.url.length() > MAX_URL_BYTES || !urlValid(station.url)) {
      error = "Sender " + String(i + 1) + ": Name (max. 63 UTF-8-Bytes) und HTTP(S)-URL (max. 255 Bytes) prüfen."; return false;
    }
    if (groupIndex(station.group) == result.groups.size()) {
      error = "Jeder Sender muss einer vorhandenen Gruppe zugeordnet sein."; return false;
    }
    if (!loading && station.id == 0) {
      if (result.nextId >= UINT32_MAX - 1) { error = "Keine Sender-IDs verfügbar."; return false; }
      station.id = result.nextId++;
    } else if ((loading && station.id >= result.nextId) || (!loading && find(station.id) == this->count())) {
      error = "Unbekannte Sender-ID. Bitte Seite neu laden."; return false;
    }
    for (const auto& previous : result.stations) if (previous.id == station.id) {
      error = "Doppelte Sender-ID."; return false;
    }
    result.stations.push_back(std::move(station));
  }
  return true;
}
String StationStore::encode(const State& state) {
  Json root(cJSON_CreateObject(), cJSON_Delete);
  if (!root || !cJSON_AddNumberToObject(root.get(), "nextId", state.nextId) ||
      !cJSON_AddNumberToObject(root.get(), "nextGroupId", state.nextGroupId) ||
      !cJSON_AddStringToObject(root.get(), "hostname", state.hostname.c_str())) return String();
  cJSON* groups = cJSON_AddArrayToObject(root.get(), "groups");
  if (!groups) return String();
  for (const auto& group : state.groups) {
    cJSON* row = cJSON_CreateObject();
    if (!row) return String();
    if (!cJSON_AddItemToArray(groups, row)) { cJSON_Delete(row); return String(); }
    if (!cJSON_AddNumberToObject(row, "id", group.id) ||
        !cJSON_AddStringToObject(row, "name", group.name.c_str()) ||
        !cJSON_AddNumberToObject(row, "parent", group.parent)) return String();
  }
  cJSON* list = cJSON_AddArrayToObject(root.get(), "stations");
  if (!list) return String();
  for (const auto& station : state.stations) {
    cJSON* row = cJSON_CreateObject();
    if (!row) return String();
    if (!cJSON_AddItemToArray(list, row)) { cJSON_Delete(row); return String(); }
    if (!cJSON_AddNumberToObject(row, "id", station.id) ||
        !cJSON_AddStringToObject(row, "name", station.name.c_str()) ||
        !cJSON_AddStringToObject(row, "url", station.url.c_str()) ||
        !cJSON_AddNumberToObject(row, "group", station.group)) return String();
  }
  char* text = cJSON_PrintUnformatted(root.get());
  if (!text) return String();
  String result(text); cJSON_free(text); return result;
}
String StationStore::json() const { return encode(state_); }
bool StationStore::replace(const String& text, String& error) {
  State replacement;
  if (!decode(text, false, replacement, error)) return false;
  const String saved = encode(replacement);
  if (saved.isEmpty() || saved.length() > MAX_JSON_BYTES) {
    error = "Die Radiokonfiguration benötigt zu viel Speicher. Bitte Namen oder URLs kürzen."; return false;
  }
  if (!ready_ || preferences_.putBytes("list", saved.c_str(), saved.length()) != saved.length()) {
    error = "Speichern fehlgeschlagen. Die bisherige Radiokonfiguration bleibt aktiv."; return false;
  }
  state_ = std::move(replacement);
  return true;
}
