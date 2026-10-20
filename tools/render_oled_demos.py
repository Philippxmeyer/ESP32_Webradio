#!/usr/bin/env python3
"""Render the real OLED menu/display code with Adafruit GFX on the host.

No network or hardware access. The default station list is loaded into a host
adapter; SWR3 song/stream metadata are deterministic demonstration values.
Only Python's standard library and g++ are needed in addition to Adafruit GFX.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import zlib

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "docs/images/oled"
SOURCES = ["RadioDisplay.cpp", "RadioDisplay.h", "StreamMetadata.cpp",
           "StreamMetadata.h", "StationMenu.h", "Stations.h", "Config.h", "OledText.h"]
FRAMES = [
    ("01-swr3-wiedergabe", "SWR3: Wiedergabe"),
    ("02-swr3-laufschrift", "SWR3: Laufschrift"),
    ("03-gruppen-pop", "Gruppen: Pop & Hits"),
    ("04-pop-swr3", "Pop: SWR3"),
    ("05-pop-wdr2", "Pop: WDR2 markieren"),
    ("06-gruppen-elektro", "Gruppen: Elektro"),
    ("07-elektro-untergruppen", "Elektro: Untergruppen"),
    ("08-sunshine-techno", "sunshine: Techno"),
    ("09-sunshine-zurueck", "sunshine: zurueck"),
]

STUBS = {
    "Arduino.h": r"""#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <string>
#define ARDUINO 10800
#define PROGMEM
using String = std::string;
class __FlashStringHelper;
inline uint32_t demoNow = 0;
inline uint32_t millis() { return demoNow; }
inline void yield() {}
inline float radians(float degrees) { return degrees * 0.017453292519943295f; }
struct SerialType { void println(const char*) {} };
inline SerialType Serial;
struct WireType { void begin(int,int) {} void setClock(int) {} };
inline WireType Wire;
""",
    "Print.h": r"""#pragma once
#include "Arduino.h"
#include <cstdarg>
class Print {
public:
  virtual ~Print() = default;
  virtual size_t write(uint8_t) = 0;
  size_t write(const uint8_t* data, size_t count) {
    size_t written=0; while(count--) written += write(*data++); return written;
  }
  size_t print(const char* text) {
    return write(reinterpret_cast<const uint8_t*>(text), strlen(text));
  }
  size_t printf(const char* format, ...) {
    char text[2048]; va_list args; va_start(args, format);
    vsnprintf(text, sizeof(text), format, args); va_end(args); return print(text);
  }
};
""",
    "Adafruit_I2CDevice.h": "#pragma once\n",
    "Adafruit_SPIDevice.h": "#pragma once\n",
    "HostDisplay.h": r"""#pragma once
#include <Adafruit_GFX.h>
inline GFXcanvas1* demoCanvas = nullptr;
class HostDisplay : public GFXcanvas1 {
public:
  HostDisplay(int16_t w, int16_t h, WireType*, int) : GFXcanvas1(w,h) {}
  bool begin(int, bool) { demoCanvas=this; return getBuffer()!=nullptr; }
  bool begin(int, int, bool, bool) { demoCanvas=this; return getBuffer()!=nullptr; }
  void clearDisplay() { fillScreen(0); }
  void display() { demoCanvas=this; }
};
""",
    "Adafruit_SH110X.h": '#pragma once\n#include "HostDisplay.h"\nusing Adafruit_SH1106G = HostDisplay;\n',
    "Adafruit_SSD1306.h": '#pragma once\n#include "HostDisplay.h"\n#define SSD1306_SWITCHCAPVCC 2\nusing Adafruit_SSD1306 = HostDisplay;\n',
    "RadioAudio.h": '#pragma once\n#include "StreamMetadata.h"\nenum class AudioStatus { WaitingWifi, Connecting, Playing, Retry, Error };\n',
    "StationStore.h": r"""#pragma once
#include "Stations.h"
#include <vector>
struct SavedStation { uint32_t id; String name, url; uint32_t group; };
struct SavedGroup { uint32_t id; String name; uint32_t parent; };
class StationStore {
  std::vector<SavedStation> stations_;
  std::vector<SavedGroup> groups_;
public:
  static constexpr size_t MAX_NAME_BYTES = 63;
  StationStore() {
    for(size_t i=0;i<STATION_COUNT;++i)
      stations_.push_back({uint32_t(i+1),STATIONS[i].name,STATIONS[i].url,uint32_t(STATIONS[i].group)});
    for(size_t i=0;i<STATION_GROUP_COUNT;++i)
      groups_.push_back({uint32_t(i),STATION_GROUPS[i].name,uint32_t(STATION_GROUPS[i].parent)});
  }
  size_t count() const { return stations_.size(); }
  size_t groupCount() const { return groups_.size(); }
  const SavedGroup& groupAt(size_t i) const { return groups_.at(i); }
  size_t findGroup(uint32_t id) const {
    for(size_t i=0;i<groups_.size();++i) if(groups_[i].id==id) return i;
    return groups_.size();
  }
  const SavedGroup& group(uint32_t id) const { return groups_.at(findGroup(id)); }
  const SavedStation& at(size_t i) const { return stations_.at(i); }
};
inline StationStore stationStore;
""",
}

MAIN = r"""#include "RadioDisplay.h"
#include "HostDisplay.h"
#include <cassert>
#include <fstream>
#include <string>

void capture(const std::string& directory, const char* name) {
  assert(demoCanvas && demoCanvas->width()==128 && demoCanvas->height()==64);
  std::ofstream out(directory+"/"+name+".pgm",std::ios::binary);
  out << "P5\n128 64\n255\n";
  for(int y=0;y<64;++y) for(int x=0;x<128;++x)
    out.put(demoCanvas->getPixel(x,y) ? char(255) : char(0));
  assert(out.good());
}
size_t chooseGroup(StationMenu& menu, StationGroupId group) {
  for(size_t i=0;i<menu.count();++i) {
    const auto entry=menu.entry(i);
    if(entry.kind==StationMenu::Kind::Group && stationStore.groupAt(entry.index).id==uint32_t(group)) {
      menu.select(i); return i;
    }
  }
  assert(false); return 0;
}
void chooseStation(StationMenu& menu, const char* name) {
  for(size_t i=0;i<menu.count();++i) {
    const auto entry=menu.entry(i);
    if(entry.kind==StationMenu::Kind::Station && !strcmp(menu.name(i),name)) {
      menu.select(i); return;
    }
  }
  assert(false);
}
int main(int argc,char** argv) {
  assert(argc==2);
  const std::string output=argv[1];
  RadioDisplay display;
  StationMenu menu;
  size_t station=0;
  assert(!strcmp(stationStore.at(station).name.c_str(),"SWR3"));
  assert(display.begin());
  StreamMetadata metadata;
  metadata.clear(stationStore.at(station).id,stationStore.at(station).url.c_str());
  metadata.streamTitle("Dua Lipa - Houdini");
  StreamMetadata::copyText(metadata.description,sizeof(metadata.description),"SWR3 Livestream");
  StreamMetadata::copyText(metadata.url,sizeof(metadata.url),"https://www.swr3.de");
  StreamMetadata::copyText(metadata.codec,sizeof(metadata.codec),"MP3");
  metadata.bitrate=128000; metadata.sampleRate=44100; metadata.channels=2;
  demoNow=0;
  display.render(station,menu,8,false,0,AudioStatus::Playing,metadata);
  capture(output,"01-swr3-wiedergabe");

  // Advance the real scroll/page timers, rather than repositioning any text.
  metadata.streamTitle("Tears for Fears - Everybody Wants To Rule The World");
  for(demoNow=1000;demoNow<=6500;demoNow+=250)
    display.render(station,menu,8,false,0,AudioStatus::Playing,metadata);
  capture(output,"02-swr3-laufschrift");

  auto menuFrame=[&](const char* name,uint32_t remaining=5000) {
    demoNow+=250;
    display.render(station,menu,8,true,remaining,AudioStatus::Playing,metadata);
    capture(output,name);
    assert(station==0); // Menu navigation keeps SWR3 playing until confirmation.
  };
  menu.reset();
  chooseGroup(menu,StationGroupId::Pop);
  menuFrame("03-gruppen-pop");
  assert(menu.activate(station)==StationMenu::Action::Navigate);
  chooseStation(menu,"SWR3");
  menuFrame("04-pop-swr3");
  chooseStation(menu,"WDR2");
  menuFrame("05-pop-wdr2",4000);
  menu.select(menu.count()-1);
  assert(menu.activate(station)==StationMenu::Action::Navigate);
  assert(uint32_t(menu.group())==uint32_t(StationGroupId::Overview));
  chooseGroup(menu,StationGroupId::Electronic);
  menuFrame("06-gruppen-elektro");
  assert(menu.activate(station)==StationMenu::Action::Navigate);
  chooseGroup(menu,StationGroupId::Sunshine);
  menuFrame("07-elektro-untergruppen");
  assert(menu.activate(station)==StationMenu::Action::Navigate);
  chooseStation(menu,"sunshine Techno");
  menuFrame("08-sunshine-techno",4000);
  menu.select(menu.count()-1);
  menuFrame("09-sunshine-zurueck",3000);
  assert(menu.activate(station)==StationMenu::Action::Navigate);
  assert(uint32_t(menu.group())==uint32_t(StationGroupId::Electronic));
  assert(stationStore.groupAt(menu.entry(menu.selected()).index).id==uint32_t(StationGroupId::Sunshine));
}
"""


def png(path, width, height, pixels):
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind+data)&0xffffffff)
    scanlines=b"".join(b"\0"+pixels[y*width:(y+1)*width] for y in range(height))
    data=b"\x89PNG\r\n\x1a\n"+chunk(b"IHDR",struct.pack(">IIBBBBB",width,height,8,0,0,0,0))
    path.write_bytes(data+chunk(b"IDAT",zlib.compress(scanlines,9))+chunk(b"IEND",b""))


def scale(pixels, width, height, factor):
    rows=[]
    for y in range(height):
        row=b"".join(bytes([p])*factor for p in pixels[y*width:(y+1)*width])
        rows.extend([row]*factor)
    return b"".join(rows)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gfx-library",type=Path,
        default=Path.home()/"Arduino/libraries/Adafruit_GFX_Library")
    args=parser.parse_args()
    gfx=args.gfx_library.resolve()
    if not (gfx/"Adafruit_GFX.cpp").is_file():
        parser.error("Adafruit GFX nicht gefunden; --gfx-library /pfad/zur/bibliothek verwenden")
    if not shutil.which("g++"):
        parser.error("g++ wird zum Rendern des echten Displaycodes benoetigt")
    OUTPUT.mkdir(parents=True,exist_ok=True)
    (OUTPUT/"native").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="webradio-oled-") as directory:
        work=Path(directory)
        for name in SOURCES:
            shutil.copy2(ROOT/name,work/name)
        for name,text in STUBS.items():
            (work/name).write_text(text)
        (work/"main.cpp").write_text(MAIN)
        command=["g++","-std=c++17","-DARDUINO=10800","-O2","-Wall","-Wextra",
                 "-I",str(work),"-I",str(gfx),str(gfx/"Adafruit_GFX.cpp"),
                 str(work/"RadioDisplay.cpp"),str(work/"StreamMetadata.cpp"),
                 str(work/"main.cpp"),"-o",str(work/"render")]
        subprocess.run(command,check=True)
        subprocess.run([str(work/"render"),str(work)],check=True)
        frames=[]
        for name,label in FRAMES:
            header,width_height,maximum,pixels=(work/(name+".pgm")).read_bytes().split(b"\n",3)
            assert (header,width_height,maximum)==(b"P5",b"128 64",b"255")
            assert len(pixels)==128*64 and set(pixels)=={0,255}
            # The border and fill of the bottom volume bar must be visible.
            assert all(pixels[59*128+x]==255 for x in range(128))
            png(OUTPUT/"native"/(name+".png"),128,64,pixels)
            png(OUTPUT/(name+".png"),512,256,scale(pixels,128,64,4))
            frames.append((name,label,pixels))

    # Contact sheet: original display pixels at 3x, with labels outside displays.
    columns,margin,gap,tile_w,tile_h=3,24,24,384,224
    width=margin*2+columns*tile_w+(columns-1)*gap
    height=margin*2+3*tile_h+2*gap
    sheet=bytearray([22])*(width*height)
    font_text=(gfx/"glcdfont.c").read_text()
    font_block=font_text.split("font[] PROGMEM = {",1)[1].split("};",1)[0]
    font=bytes(int(value,16) for value in re.findall(r"0x([0-9A-Fa-f]{2})",font_block))
    assert len(font)==256*5
    for index,(name,label,pixels) in enumerate(frames):
        left=margin+(index%3)*(tile_w+gap)
        top=margin+(index//3)*(tile_h+gap)
        large=scale(pixels,128,64,3)
        for y in range(192):
            start=(top+y)*width+left
            sheet[start:start+384]=large[y*384:(y+1)*384]
        for column,char in enumerate(label):
            for x in range(5):
                bits=font[ord(char)*5+x]
                for y in range(8):
                    if bits&(1<<y):
                        for dy in range(2):
                            for dx in range(2):
                                px=left+column*12+x*2+dx
                                py=top+204+y*2+dy
                                sheet[py*width+px]=230
    png(OUTPUT/"uebersicht.png",width,height,bytes(sheet))
    hashes={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in SOURCES}
    for name in ["Adafruit_GFX.cpp","Adafruit_GFX.h","glcdfont.c","gfxfont.h"]:
        hashes["Adafruit_GFX/"+name]=hashlib.sha256((gfx/name).read_bytes()).hexdigest()
    manifest={"display_pixels":[128,64],"enlarged_pixels":[512,256],
        "method":"Actual RadioDisplay.cpp, StationMenu.h, StreamMetadata.cpp and Adafruit GFX / GFXcanvas1",
        "data":"Default station list; simulated time; demonstration SWR3 song/stream metadata; no hardware or live stream capture",
        "source_sha256":hashes,"frames":[{"file":name+".png","caption":label} for name,label in FRAMES]}
    (OUTPUT/"manifest.json").write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+"\n")
    print(f"{len(FRAMES)} OLED-Screens, native Kopien und Uebersicht gerendert: {OUTPUT}")


if __name__=="__main__":
    main()
