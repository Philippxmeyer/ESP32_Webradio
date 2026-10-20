#include "RadioWeb.h"
#include "StationStore.h"
#include "WebPage.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_system.h>
#include <cJSON.h>
#include <memory>
namespace { using Json = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>; }
void RadioWeb::headers() {
  server_.sendHeader("Cache-Control", "no-store");
  server_.sendHeader("X-Content-Type-Options", "nosniff");
  server_.sendHeader("X-Frame-Options", "DENY");
  server_.sendHeader("Referrer-Policy", "no-referrer");
  server_.sendHeader("Content-Security-Policy", "default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'; form-action 'self'; base-uri 'none'");
}
void RadioWeb::begin(std::function<void()> changed) {
  changed_ = std::move(changed);
  activeHostname_ = stationStore.hostname();
  char token[33];
  snprintf(token, sizeof(token), "%08lx%08lx%08lx%08lx", (unsigned long)esp_random(),
           (unsigned long)esp_random(), (unsigned long)esp_random(), (unsigned long)esp_random());
  token_ = token;
  revision_ = esp_random();
  const char* keys[] = {"X-Radio-Token", "Content-Type"};
  server_.collectHeaders(keys, 2);
  server_.on("/", HTTP_GET, [this] {
    headers(); server_.send_P(200, "text/html; charset=utf-8", WEB_PAGE);
  });
  server_.on("/api/stations", HTTP_GET, [this] { readList(); });
  server_.on("/api/stations", HTTP_POST, [this] { saveList(); }, [this] { receiveBody(); });
  server_.onNotFound([this] { error(404, "Seite nicht gefunden."); });
  server_.begin();
}
void RadioWeb::update() {
  if (restartPending_ && uint32_t(millis() - restartSince_) >= 1500) { ESP.restart(); return; }
  const bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasConnected_) {
    Serial.print("Senderverwaltung: http://"); Serial.println(WiFi.localIP());
    mdns_ = MDNS.begin(activeHostname_.c_str());
    if (mdns_) { MDNS.addService("http", "tcp", 80); Serial.println("http://" + activeHostname_ + ".local"); }
  } else if (!connected && wasConnected_ && mdns_) { MDNS.end(); mdns_ = false; }
  wasConnected_ = connected;
  if (connected) server_.handleClient();
}
void RadioWeb::error(int status, const String& message) {
  headers();
  Json root(cJSON_CreateObject(), cJSON_Delete);
  if (root) cJSON_AddStringToObject(root.get(), "error", message.c_str());
  char* text = root ? cJSON_PrintUnformatted(root.get()) : nullptr;
  server_.send(status, "application/json; charset=utf-8", text ? text : "{\"error\":\"Speicher knapp\"}");
  cJSON_free(text);
}
void RadioWeb::readList() {
  Json root(cJSON_Parse(stationStore.json().c_str()), cJSON_Delete);
  if (!root) { error(503, "Zu wenig freier Speicher. Bitte erneut versuchen."); return; }
  bool ok = cJSON_AddNumberToObject(root.get(), "revision", revision_) &&
    cJSON_AddStringToObject(root.get(), "token", token_.c_str()) &&
    cJSON_AddNumberToObject(root.get(), "maxStations", StationStore::MAX_STATIONS) &&
    cJSON_AddNumberToObject(root.get(), "maxBytes", StationStore::MAX_JSON_BYTES);
  ok = cJSON_AddNumberToObject(root.get(), "maxGroups", StationStore::MAX_GROUPS) &&
    cJSON_AddStringToObject(root.get(), "activeHostname", activeHostname_.c_str()) &&
    cJSON_AddBoolToObject(root.get(), "restarting", restartPending_) &&
    cJSON_AddStringToObject(root.get(), "ipAddress", WiFi.localIP().toString().c_str()) && ok;
  char* text = ok ? cJSON_PrintUnformatted(root.get()) : nullptr;
  if (!text) { error(503, "Zu wenig freier Speicher. Bitte erneut versuchen."); return; }
  headers(); server_.send(200, "application/json; charset=utf-8", text);
  cJSON_free(text);
}
void RadioWeb::receiveBody() {
  if (!server_.hasRaw()) return;
  const auto& raw = server_.raw();
  if (raw.status == RAW_START) { body_ = ""; oversized_ = false; complete_ = false; }
  else if (raw.status == RAW_WRITE && !oversized_) {
    if (body_.length() + raw.currentSize > StationStore::MAX_JSON_BYTES ||
        !body_.concat(reinterpret_cast<const char*>(raw.buf), raw.currentSize)) {
      oversized_ = true; body_ = "";
    }
  } else if (raw.status == RAW_END) complete_ = true;
  else if (raw.status == RAW_ABORTED) { complete_ = false; body_ = ""; }
}
void RadioWeb::saveList() {
  if (restartPending_) { body_ = ""; error(409, "Das Radio startet neu. Bitte danach die neue Adresse öffnen."); return; }
  if (server_.header("X-Radio-Token") != token_) { body_ = ""; error(403, "Bitte Seite neu laden und erneut speichern."); return; }
  if (!server_.header("Content-Type").startsWith("application/json")) { body_ = ""; error(415, "JSON erwartet."); return; }
  if (oversized_) { error(413, "Die Senderliste ist zu groß (max. 8192 Bytes). Bitte Namen oder URLs kürzen."); return; }
  if (!complete_) { body_ = ""; error(400, "Unvollständige Anfrage."); return; }
  // The query version avoids parsing an unvalidated body a second time.
  const String expected = String(revision_);
  const String supplied = server_.arg("revision");
  if (supplied != expected) { body_ = ""; error(409, "Die Liste wurde inzwischen geändert oder das Radio neu gestartet. Bitte neu laden; lokale Änderungen bleiben bis dahin sichtbar."); return; }
  String message;
  const bool saved = stationStore.replace(body_, message);
  body_ = ""; complete_ = false;
  if (!saved) { error(400, message); return; }
  ++revision_;
  if (changed_) changed_();
  if (stationStore.hostname() != activeHostname_) { restartPending_ = true; restartSince_ = millis(); }
  readList();
}
