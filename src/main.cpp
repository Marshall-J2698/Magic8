#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_ADXL345_U.h>
#include "Free_Fonts.h"

#define DEBUG 0

#define DEF_BLUE 0x0027

const long MINDELAY = 5000;
const long HOLDFOR = 4000;
const float SHAKETHRESHOLD = 22;
float Ax,Ay,Az,mag;

TFT_eSPI tft = TFT_eSPI();

int xpos, ypos;

char projects [250][70] = {"0008-Facemask Tutorial", "0017-Crescent flower earrings", "3D-Printers:Ender3", "3D-Printers:Kili", "3D-Printers:Thorin", "3D Modeling with Blender", "3D Scanner", "3D printed bobbin", "AM radio transmitter", "A Simple Wenner Array", "A Sound (Frequency) Detection Circuit", "A simple electrocardiogram circuit", "Acoustic to Electrical instrument converter", "Airbrush", "Amigurumi", "Analog Modular Synth", "Analog Modular Synthesizer", "Arbor Press", "Arm Warmers/Covers from Scrap Fabric", "Automatic Desk Lamp Concept", "Beginner Crochet", "Beginner knitting", "Belt Sander", "Bike Phone Mount", "Bike Repair Cart", "Bootleg Snowskate", "Bosch CM10GD", "Bosch CM10GD Miter Saw", "Box Cutter", "Building a Transistor Theremin", "Buildspot Machine Shop", "Buildspot Wood Shop", "Button Maker", "Button Maker Tutorial", "CNC Mill", "CNC Plasma Cutting", "Cal Poly SLO Mustang '60 Machine Shop", "Carl-o-gotchi", "Cassette Player Circuit", "Clamp lab", "Class D Operational Amplifier", "Class of 1969 Makerspace", "Cleaning The GlowForge", "Clothing Repair", "Clothing Repair Tools", "Color Lamp", "Concrete", "Crescent flower earrings", "Cricut Maker 3", "Crochet, Kit", "Crochet (Kit)", "Crochet Hyperbolic Pseudospheres", "Crocheted Flowers", "DCS312", "DCS334", "DCS380", "DIY Chipmunk", "DeWalt DCN660", "DeWalt DCN680", "DeWalt DCN681", "DeWalt DCS512", "DeWalt DCS520", "DeWalt DCS577", "DeWalt DCW210", "DeWalt DW735", "Delta 66-120", "Demo", "Drill", "Drill Press", "ESP32:Intro", "ESP32 WROOM", "Ender 3 Max Neo", "Engineering Product Innovation Center (EPIC)", "EpoxyResin", "Epoxy Resin", "Extech X503 Multimeter", "FM Oscillator and Radio Frequency Amplifier Circuit", "Facemask Tutorial", "Filament deposition modeling printer", "Flow Mach 2 Waterjet", "Ford Campus Wood Shop", "Formlabs Form 3 3D Printer", "Formlabs Fuse 1 3D Printer", "Frequency Modulator", "Friday Flowers Crochet", "Fusion360", "GALLERY", "Glowforge Pro", "Hardinge Toolroom Lathe", "Harrison Lathe", "Headphone Guitar Amp with Tremelo", "Herb Garden", "Home", "Home New", "Horizontal Bandsaw", "Impact Driver", "Intro Raspberry Pi", "JET JJB6", "Jet JJB6", "Kili (3D Printer)", "Kitchen Lithography", "LED Audio Visualizer", "LED Music Visualizer", "Laguna 14bx", "Laser Cut Catan Board", "Laser Engraver", "Leaderboard", "LulzBot 3D Printer", "MIG Welder", "Machine Shop Tools", "Macropad", "Main Page", "Make your own deodorant and toothpaste!", "Maker Tracking System", "Makerspace Instrument Shop", "Makerspace Machine Shop", "Makerspace Repair Lair", "Makerspace Tools", "Makerspace Woodshop", "Metal Detector", "Metalworking Lathes", "Mic stand", "Microcontrollers:ESP32", "Milwaukee 6480-20", "Mini Electric Generator", "Mini Soldering Hoods", "Module:ProjectGallery", "Mushroom Boi Earrings", "Nametag", "Nomai Mask - EVA foam", "Original Prusa XL 3D Printer", "Osaka Castle Model", "Over-Complicated Digital Thermometer", "Patch Making out of Scraps and Acrylic Paints", "Patches", "Photogrammetry", "Pi Basics", "Plasma Cutter (CNC)", "Pompoms!", "Powermatic PM2800B", "Project", "ProjectLabels", "Project Tutorials", "Project Tutorials test", "Projects:ccms-0013", "Projects:ccms-0014", "Projects:ccms-0015", "Projects:ccms-0016", "Projects:ccms-0017", "Punch Press", "Quest0004", "Rapid Prototyping Lab", "Raspberry Pi:Intro", "Reprovisioned Press", "Rigol Oscilloscope", "SBU CEAS Shops", "Sample Create", "Sample SSMC Space", "Sandbox", "SawStop ICS31230-36", "Screenprinting", "Sewing Arm Warmers", "Sewing Cart", "Sewing Leg Warmers", "Sewing Machine", "Shrek Bucket Hat", "Software:Fusion360", "Soldering Cart", "Spaces", "Spaces:StudentMachineShop", "Spaces New", "Spray Booth", "Stickers", "Student Machine Shop", "Stylophone", "TIG Welder", "TLC Plate Cutting Rig", "TOOLS", "TRAK DPM2 Mill", "Temperature & Relative Humidity & Light Sensor with an Alarm", "Test", "Test", "Test 2023", "Testamundo", "Testing", "The Coolest Project Possible", "The Electronics of a Stylophone", "Thorin", "Todd and Lemon", "Tools", "Tools:Laser Engraver", "Tools test", "Try", "Tube Sound Fuzz Guitar Effect Pedal", "Types of Stitches", "Univeral PLS6.75", "Universal PLS6.75", "UserStats", "VF4SS. VF2SS w/ Trunnion", "Vanilla Latte Socks", "VerticalBandsaw", "VerticalMill", "Vertical Bandsaw", "Vertical Mill", "Weller WES50", "Wood Shop Tools", "Zener Diode Noise Random Number Generator"};


enum ScreenStates{
  IDLE,
  SHAKEN
};

enum ScreenStates curScreen = IDLE;


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
unsigned long last_fire = 0;

#define HOR_RES 240
#define VER_RES 240

void setup() {

  xpos = 120;
  ypos = 120;
  accel.begin();
  accel.setRange(ADXL345_RANGE_16_G);
  Serial.begin();
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(10);
  sensors_event_t event;
  accel.getEvent(&event);
  Ax = event.acceleration.x;
  Ay = event.acceleration.y;
  Az = event.acceleration.z;
  mag = sqrt(pow(Ax,2) + pow(Ay,2)+ pow(Az,2));
  if (millis()-last_fire > MINDELAY && mag > SHAKETHRESHOLD){
    delay(1000);
    tft.setFreeFont(FSB24);
    tft.setTextDatum(MC_DATUM);
    
    int ind = random(0,200);
    for(int i = 0; i < 256; i+=2){
      tft.fillTriangle(120,16,30,172,210,172,tft.alphaBlend(i,DEF_BLUE,TFT_BLACK));
      if (i > 120){
        tft.setTextColor(tft.alphaBlend(i,TFT_WHITE,TFT_BLACK));
      tft.drawString(projects[ind],xpos,ypos,GFXFF); 
      }
      
    }
;
    


    curScreen = SHAKEN;
    last_fire = millis();
  }

  if (DEBUG){
    Serial.print(Ax);
    Serial.print(',');
    Serial.print(Ay);
    Serial.print(',');
    Serial.print(Az);
    Serial.print(',');
    Serial.print("mag: ");
    Serial.print(mag);
    Serial.println();
  }

  if (millis()-last_fire > HOLDFOR && curScreen == SHAKEN){
    curScreen = IDLE;
    for(int i = 0; i < 256; i+=2){
      tft.fillTriangle(120,16,30,172,210,172,tft.alphaBlend(i,TFT_BLACK,DEF_BLUE));
    }
    tft.fillScreen(TFT_BLACK);
  }

}



