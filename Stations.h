#ifndef WEBRADIO_STATIONS_H
#define WEBRADIO_STATIONS_H
#include <Arduino.h>
// Parents define the menu tree; keep the overview first and enum/table in sync.
enum class StationGroupId : uint8_t {
  Overview, Pop, Rock, Relax, Family, Discovery, News, Electronic, English,
  Sunshine, BigFM, Count
};
struct StationGroup { const char* name; StationGroupId parent; };
static const StationGroup STATION_GROUPS[] = {
  {"GRUPPEN", StationGroupId::Overview},
  {"Pop & Hits", StationGroupId::Overview},
  {"Rock", StationGroupId::Overview},
  {"Entspannung & Jazz", StationGroupId::Overview},
  {"Kinder & Familie", StationGroupId::Overview},
  {"Musik entdecken", StationGroupId::Overview},
  {"Nachrichten & Kultur", StationGroupId::Overview},
  {"Elektro & Dance", StationGroupId::Overview},
  {"Englischsprachig", StationGroupId::Overview},
  {"sunshine live", StationGroupId::Electronic},
  {"bigFM", StationGroupId::Electronic}
};
constexpr size_t STATION_GROUP_COUNT = sizeof(STATION_GROUPS) / sizeof(STATION_GROUPS[0]);
static_assert(STATION_GROUP_COUNT == size_t(StationGroupId::Count), "Group enum/table mismatch");
struct Station { const char* name; const char* url; StationGroupId group; };
// Stored in flash. Direct MP3/AAC streams or M3U/PLS URLs.
static const Station STATIONS[] = {
  {"SWR3", "https://liveradio.swr.de/sw282p3/swr3/play.mp3", StationGroupId::Pop},
  {"Rockantenne", "https://stream.rockantenne.de/rockantenne/stream/mp3", StationGroupId::Rock},
  {"Radio Bob!", "https://streams.radiobob.de/bob-national/mp3-192/mediaplayer", StationGroupId::Rock},
  {"Absolut Relax", "https://absolut-relax.live-sm.absolutradio.de/absolut-relax/stream/mp3", StationGroupId::Relax},
  {"Radio Mixtape", "https://stream.lokalradio.nrw/4459jsj", StationGroupId::Pop},
  {"WDR2", "https://wdr-wdr2-rheinland.icecastssl.wdr.de/wdr/wdr2/rheinland/mp3/128/stream.mp3", StationGroupId::Pop},
  {"RPR1", "https://stream.rpr1.de/webradio/mp3-128/private", StationGroupId::Pop},
  {"N-Joy", "http://icecast.ndr.de/ndr/njoy/live/mp3/128/stream.mp3", StationGroupId::Pop},
  // UTF-8 names are converted to CP437 only for OLED output.
  {"Ö3", "https://orf-live.ors-shoutcast.at/oe3-q1a", StationGroupId::Pop},

  // Children and family.
  {"MausRadio", "https://wdr-diemaus-live.icecastssl.wdr.de/wdr/diemaus/live/mp3/56/stream.mp3", StationGroupId::Family},
  {"Radio TEDDY", "https://streamtdy.ir-media-tec.com/live/mp3-128/web/play.mp3", StationGroupId::Family},
  {"TOGGO Radio", "https://superrtl.streamabc.net/srtl-toggo-mp3-192-3253152", StationGroupId::Family},

  // Music discovery and relaxed listening.
  {"ByteFM", "https://www.byte.fm/live/mp3-128/website", StationGroupId::Discovery},
  {"radioeins", "https://dispatcher.rndfnk.com/rbb/radioeins/live/mp3/mid", StationGroupId::Discovery},
  {"Radio Paradise", "https://stream.radioparadise.com/mp3-128", StationGroupId::Relax},
  {"Radio Swiss Jazz", "https://stream.srg-ssr.ch/srgssr/rsj/mp3/128", StationGroupId::Relax},

  // News, background and culture.
  {"Deutschlandfunk", "https://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3", StationGroupId::News},
  {"Dlf Kultur", "https://st02.sslstream.dlf.de/dlf/02/128/mp3/stream.mp3", StationGroupId::News},
  {"Dlf Nova", "https://st03.sslstream.dlf.de/dlf/03/128/mp3/stream.mp3", StationGroupId::News},
  {"tagesschau24", "https://icecast.tagesschau.de/ndr/tagesschau24/live/mp3/128/stream.mp3", StationGroupId::News},

  // Electronic music.
  {"sunshine live", "https://stream.sunshine-live.de/live/mp3-128/stream.sunshine-live.de/", StationGroupId::Sunshine},
  {"sunshine Techno", "https://stream.sunshine-live.de/techno/mp3-128/stream.sunshine-live.de/", StationGroupId::Sunshine},
  {"sunshine Classics", "https://stream.sunshine-live.de/classics/mp3-128/stream.sunshine-live.de/", StationGroupId::Sunshine},
  {"bigFM House Beats", "https://stream.bigfm.de/deeptechhouse/mp3-128/stream.bigfm.de/", StationGroupId::BigFM},
  {"bigFM Dance", "https://stream.bigfm.de/dance/mp3-128/stream.bigfm.de/", StationGroupId::BigFM},

  // English-language stations (Radio Paradise is listed above).
  {"Radio X UK", "https://media-ice.musicradio.com/RadioXUKMP3", StationGroupId::English},
  {"Radio X Classic", "https://media-ice.musicradio.com/RadioXClassicRockMP3", StationGroupId::English},
  {"KEXP Seattle", "https://kexp.streamguys1.com/kexp160.aac", StationGroupId::English},
  {"AFN Stuttgart", "https://playerservices.streamtheworld.com/api/livestream-redirect/AFNE_STU.mp3", StationGroupId::English}
};
constexpr size_t STATION_COUNT = sizeof(STATIONS) / sizeof(STATIONS[0]);
static_assert(STATION_COUNT > 0, "At least one station is required");

#endif

