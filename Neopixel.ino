#include <Adafruit_NeoPixel.h>


#define NeoPixelStartupAnimationActive true  //Show Startup Animation for all Neopixels (true = activated / false = deactivated) !!Attention!! Animation will only be played if all NeoPixels have the same number of LEDs
#define COLOR(r, g, b) (((uint32_t)r << 16) | ((uint32_t)g <<  8) | b)

#define NumberNeoPixels 3

struct StructNeoPixelConfig {
  bool Active;
  uint32_t StartupAnimationColor;
  neoPixelType Type;
  uint16_t LEDs;
  uint16_t Pin;
  int8_t PixelOffset;
  float TempOffset;
  bool AnimationActive;
  bool AnimationReverse;
  uint8_t Brightness;
  bool AnimationMemoryActive;
  bool AnimationMemoryRunning;
  uint8_t AnimationMemoryPosition;
  uint8_t AnimationMemoryPosition_Memory;
  uint8_t AnimationMemoryRangeBegin;
  uint8_t AnimationMemoryRangeEnd;
  uint8_t AnimationMemoryAnimationColor;
};

StructNeoPixelConfig NeoPixelConfig[NumberNeoPixels] = {
  { // Neopixel 1
    Active                                : true,
    StartupAnimationColor                 : COLOR(0, 0, 255),
    Type                                  : NEO_GRB + NEO_KHZ800,
    LEDs                                  : 16,
    Pin                                   : 7,
    PixelOffset                           : 2,
    TempOffset                            : 0,
    AnimationActive                       : true,
    AnimationReverse                      : false,
    Brightness                            : 8,
  },
  { // Neopixel 2
    Active                                : true,
    StartupAnimationColor                 : COLOR(255, 0, 0),
    Type                                  : NEO_GRB + NEO_KHZ800,
    LEDs                                  : 16,
    Pin                                   : 6,
    PixelOffset                           : 0,
    TempOffset                            : 0,
    AnimationActive                       : true,
    AnimationReverse                      : false,
    Brightness                            : 8,
  },
  { // Neopixel 3
    Active                                : true,
    StartupAnimationColor                 : COLOR(0, 255, 0),
    Type                                  : NEO_GRB + NEO_KHZ800,
    LEDs                                  : 16,
    Pin                                   : 8,
    PixelOffset                           : 0,
    TempOffset                            : 0,
    AnimationActive                       : true,
    AnimationReverse                      : false,
    Brightness                            : 8,
  },
};


#define COLOR1(x) COLOR(x) // expand a single argument to 3 for COLOR

#define NeopixelRefreshSpeed 200

unsigned long NeoPixelTimerRefresh = millis();
uint8_t NeoPixelID;
uint8_t NeoPixelLEDID;

Adafruit_NeoPixel *NeoPixel_Device[NumberNeoPixels];

static int ConvertPosition2PixelIndex(int PixelCount, int PixelOffset, int Position, bool ReverseDirection) {
  int newposition;
  int newpixeloffset;
  if (ReverseDirection == false) {
    newpixeloffset = PixelOffset;
  }
  else{
    newpixeloffset = -PixelOffset;
  }
  if ((Position + newpixeloffset) > PixelCount) {
    newposition = (Position + newpixeloffset) - PixelCount;
  }
  else if ((Position + newpixeloffset) < 1) {
    newposition = PixelCount + (Position + newpixeloffset);
  }
  else {
    newposition = (Position + newpixeloffset);
  }  
  if (ReverseDirection == false) {
    return (PixelCount - newposition);
  }
  else {  
    return (newposition - 1);
  }  
}

// Initialize everything and prepare to start
void setup()
{
}

// Main loop
void loop()
{
   
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize Variables
  int16_t BrightnessID = 0;
  uint8_t NeoPixelAnimationStep;
  uint8_t NeoPixelAnimationCount;
  uint8_t NeoPixelAnimationID;
  uint32_t AnimiationColor[NumberNeoPixels];
  
  // Initialize Neopixels
  for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++){
    if (NeoPixelConfig[NeoPixelID].Active == true) {
      NeoPixel_Device[NeoPixelID] = new Adafruit_NeoPixel(NeoPixelConfig[NeoPixelID].LEDs, NeoPixelConfig[NeoPixelID].Pin, NeoPixelConfig[NeoPixelID].Type);      
      NeoPixel_Device[NeoPixelID]->begin(); 
      NeoPixel_Device[NeoPixelID]->show();
    }
  }  

  //Set start values
  for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++){
    NeoPixelConfig[NeoPixelID].AnimationMemoryPosition = 0;
    NeoPixelConfig[NeoPixelID].AnimationMemoryPosition_Memory = 0;
    NeoPixelConfig[NeoPixelID].AnimationMemoryRangeBegin = 0;
    NeoPixelConfig[NeoPixelID].AnimationMemoryRangeEnd = 0;
    NeoPixelConfig[NeoPixelID].AnimationMemoryRunning = false;
    NeoPixelConfig[NeoPixelID].AnimationMemoryAnimationColor = 0;
  }  
  
  // Startup-Animation
  int NeoPixelCount = 0;
  bool NeoPixelStartupAnimationActive_Consistency = true;
  if (NeoPixelStartupAnimationActive == true)
  {
    // Animation Consistency-Check
    for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++){
      if (NeoPixelConfig[NeoPixelID].Active == true) {
        if (NeoPixelCount == 0) {
          NeoPixelCount = NeoPixelConfig[NeoPixelID].LEDs;
        }  
        else {
          if (NeoPixelConfig[NeoPixelID].LEDs != NeoPixelCount) {
            NeoPixelStartupAnimationActive_Consistency = false;
          }
        }
      }
    } 
    //Activate startup sequence & Co. if all activated NeoPixels have the same number of LEDs
    if (NeoPixelCount > 0 && NeoPixelStartupAnimationActive_Consistency == true) {

      //Initialize Neopixels && Build AnimationColor-Array
      NeoPixelAnimationCount = 0;
      for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++){
        if (NeoPixelConfig[NeoPixelID].Active == true) { 
          NeoPixel_Device[NeoPixelID]->setBrightness(255);
          AnimiationColor[NeoPixelAnimationCount] = NeoPixelConfig[NeoPixelID].StartupAnimationColor;
          NeoPixelAnimationCount++;
        }  
      }

      //Startup Animation Phase #1
      for (NeoPixelAnimationStep = 0; NeoPixelAnimationStep < NeoPixelAnimationCount; NeoPixelAnimationStep++) {
        for (NeoPixelLEDID = 0; NeoPixelLEDID < NeoPixelCount; NeoPixelLEDID++) {
          NeoPixelAnimationID = NeoPixelAnimationStep;
          for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++) {
            if (NeoPixelConfig[NeoPixelID].Active == true) {
              NeoPixel_Device[NeoPixelID]->fill(COLOR(0, 0, 0));
              NeoPixel_Device[NeoPixelID]->setPixelColor(ConvertPosition2PixelIndex(NeoPixelConfig[NeoPixelID].LEDs,NeoPixelConfig[NeoPixelID].PixelOffset,(NeoPixelLEDID + 1), NeoPixelConfig[NeoPixelID].AnimationReverse), AnimiationColor[NeoPixelAnimationID]); 
              NeoPixel_Device[NeoPixelID]->show();
            }  
            NeoPixelAnimationID++;
            if (NeoPixelAnimationID >= NeoPixelAnimationCount) {
              NeoPixelAnimationID = 0;
            }  
          }
          delay(30);
        }
      }  

      //Startup Animation Phase #2
      for (NeoPixelAnimationStep = 0; NeoPixelAnimationStep < NeoPixelAnimationCount; NeoPixelAnimationStep++) {
        //Part A
        for(BrightnessID = 0; BrightnessID < 255; BrightnessID++) {
          NeoPixelAnimationID = NeoPixelAnimationStep;
          for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++) {
            if (NeoPixelConfig[NeoPixelID].Active == true) {
              NeoPixel_Device[NeoPixelID]->setBrightness(BrightnessID); 
              NeoPixel_Device[NeoPixelID]->fill(AnimiationColor[NeoPixelAnimationID]);
              NeoPixel_Device[NeoPixelID]->show();
            }
            NeoPixelAnimationID++;
            if (NeoPixelAnimationID >= NeoPixelAnimationCount) {
              NeoPixelAnimationID = 0;
            }
          }
          delay(1);
        }  
        //Part B
        for(BrightnessID = 255; BrightnessID >= 0; BrightnessID--) {
          NeoPixelAnimationID = NeoPixelAnimationStep;
          for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++) {
            if (NeoPixelConfig[NeoPixelID].Active == true) {
              NeoPixel_Device[NeoPixelID]->setBrightness(BrightnessID); 
              NeoPixel_Device[NeoPixelID]->fill(AnimiationColor[NeoPixelAnimationID]);
              NeoPixel_Device[NeoPixelID]->show();
            }
            NeoPixelAnimationID++;
            if (NeoPixelAnimationID >= NeoPixelAnimationCount) {
              NeoPixelAnimationID = 0;
            }
          }
          delay(1);
        }  
      }  
    } 
  }  
  
  //Initial Neopixel for Loop
  for (NeoPixelID = 0; NeoPixelID < NumberNeoPixels; NeoPixelID++){
    if (NeoPixelConfig[NeoPixelID].Active == true) {
      NeoPixel_Device[NeoPixelID]->clear(); 
      NeoPixel_Device[NeoPixelID]->setBrightness(NeoPixelConfig[NeoPixelID].Brightness); 
      NeoPixel_Device[NeoPixelID]->show();
    }
  }
  }
