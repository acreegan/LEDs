#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

// NeoPixel settings
#define LED_PIN 6
#define LED_COUNT 12


// IR remote control settings
#define IR_PIN 2

// Fading settings
#define FADE_STEP 1
#define FADE_INTERVAL 10 // in milliseconds

// Commands
#define RED 0x45
#define GREEN 0x46
#define BLUE 0x47
#define FADE_MAGENTA 0x44
#define FADE_CYAN 0x40
#define FADE_MULTI 0x43
#define OFF 0x19


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);

unsigned long previousMillis = 0;
int brightness = 1;
int fadeDirection = 1;

uint16_t command;

struct HSVColor {
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;
};

class LedFader {
    public:
    HSVColor* colorList;
    int colorListLength;
    int delayInterval;
    float fadeTime;
    LedFader(HSVColor* colorList, int colorListLength, int fadeTime = 2550, int delayInterval=10) 
             :colorList(colorList), colorListLength(colorListLength), 
             fadeTime(fadeTime), delayInterval(delayInterval) {}

    void fadeLEDs(){
        if (intervalDelay()) {
            unsigned long elapsedTime = millis() - stateStart;
            HSVColor startColor = colorList[currentIndex];
            HSVColor targetColor = colorList[nextIndex];
            HSVColor color = gradient(startColor, targetColor, elapsedTime/fadeTime);
            strip.fill(strip.gamma32(strip.ColorHSV(color.hue, color.saturation, color.value)));
            strip.show();

            // Update color index
            if (elapsedTime >= fadeTime) {

                // Go up then back down color list
                if (nextIndex > currentIndex) {
                    // Going up
                    if (nextIndex == colorListLength-1) {
                        // Reached the top
                        currentIndex = nextIndex;
                        nextIndex--;
                    } else {
                        currentIndex = nextIndex;
                        nextIndex ++;
                    }
                } else {
                    // Going down
                    if (nextIndex == 0) {
                        // Reached the bottom
                        currentIndex = nextIndex;
                        nextIndex++;
                    } else {
                        currentIndex = nextIndex;
                        nextIndex --;
                    }
                }
                stateStart = millis();
            }
        }
    }

    void reset(){
        currentIndex = 0;
        nextIndex = 1;
        stateStart = millis();
    }

    private:
    // Calculate a point between two brightnesses
    HSVColor gradient(HSVColor startColor, HSVColor targetColor, float proportion) {

        uint16_t hue;
        if (targetColor.hue - startColor.hue < startColor.hue - targetColor.hue) {
            hue = startColor.hue + int((targetColor.hue-startColor.hue)*proportion);
        } else {
            hue = startColor.hue - int((startColor.hue-targetColor.hue)*proportion);
        }   
        int saturation = startColor.saturation + int((targetColor.saturation-startColor.saturation)*proportion);
        int brightness = startColor.value + int((targetColor.value-startColor.value)*proportion);

        HSVColor color = {hue, saturation, brightness};

        return color;
    }
    bool intervalDelay() {
        unsigned long currentMillis = millis();
        static unsigned long previousMillis = 0;
        if (currentMillis - previousMillis >= delayInterval) {
            previousMillis = currentMillis;
            return true;
        } else {
            return false;
        }
    }

    int currentIndex = 0;
    int nextIndex = 1;
    unsigned long stateStart = 0;
};

bool intervalDelay(int interval) {
    unsigned long currentMillis = millis();
    static unsigned long previousMillis = 0;
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        return true;
    } else {
        return false;
    }
}

HSVColor fadeCyan[2] = {{32768, 255, 1}, {32768, 255, 255}};
HSVColor fadeMagenta[2] = {{54613, 255, 1}, {54613, 255, 255}};
HSVColor multiColorFade[3] = {{0, 255, 150}, {8000, 255, 150}, {30000, 255, 150}};

LedFader cyanFader(fadeCyan,2);
LedFader magentaFader(fadeMagenta,2);
LedFader multiColorFader(multiColorFade, 3);

void setup() {
  strip.begin();
  strip.show(); // initialize all pixels to off

  irrecv.enableIRIn(); // start receiving IR signals

  Serial.begin(115200);
}

void loop() {
  // Check for IR signals
  if (irrecv.decode()) {
    // Handle the IR signal here
    irrecv.resume(); // receive the next value
    irrecv.printIRResultShort(&Serial);
    switch(irrecv.decodedIRData.command) {
    case RED:
        command = RED;
        break;
    case GREEN:
        command = GREEN;
        break;
    case BLUE:
        command = BLUE;
        break;
    case FADE_MAGENTA:
        command = FADE_MAGENTA;
        magentaFader.reset();
        break;
    case FADE_CYAN:
        command = FADE_CYAN;
        cyanFader.reset();
        break;
    case FADE_MULTI:
        command = FADE_MULTI;
        multiColorFader.reset();
        break;
    case OFF:
        command = OFF;
        break;
    default:
        break;
    }
  }

  switch(command){
  case RED:
    if (intervalDelay(FADE_INTERVAL)){
        strip.fill(strip.Color(255,0,0));
        strip.show();
    }
    break;
  case GREEN:
    if (intervalDelay(FADE_INTERVAL)){
        strip.fill(strip.Color(0,255,0));
        strip.show();
    }
    break;
  case BLUE:
    if (intervalDelay(FADE_INTERVAL)){
        strip.fill(strip.Color(0,0,255));
        strip.show();
    }
    break;
  case OFF:
    if (intervalDelay(FADE_INTERVAL)){
        strip.clear();
        strip.show();
    }
    break;
  case FADE_MAGENTA:
    magentaFader.fadeLEDs();
    break;
  case FADE_CYAN:
    cyanFader.fadeLEDs();
    break;
  case FADE_MULTI:
    multiColorFader.fadeLEDs();
    break;
  default:
    break;
  }

}