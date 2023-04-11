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

// Colors
#define REDHUE uint16_t((0./360) * 65535)
#define ORANGEHUE uint16_t((30./360) * 65535)
#define YELLOWHUE uint16_t((60./360) * 65535)
#define GREENHUE uint16_t((120./360) * 65535)
#define CYANHUE uint16_t((180./360) * 65535)
#define BLUEHUE uint16_t((240./360) * 65535)
#define MAGENTAHUE uint16_t((300./360) * 65535)

#define PI 3.14159265358979323846

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
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
    float holdTime;

    LedFader(HSVColor* colorList, int colorListLength, int fadeTime = 2000, 
            int holdTime = 0, int delayInterval=10) 
             :colorList(colorList), colorListLength(colorListLength), fadeTime(fadeTime), 
             holdTime(holdTime), delayInterval(delayInterval) {
                reset();
             }

    void update() {
        if (intervalDelay()) {
            elapsedTime = millis() - stateStart;
            switch(state){
            case Fade:
                fadeLEDs();
                if (elapsedTime >= fadeTime) {
                    incrementList();
                    state=Hold;
                    stateStart = millis();
                }
                break;
            case Hold:
                HSVColor color = colorList[currentIndex];
                strip.fill(strip.gamma32(strip.ColorHSV(color.hue, color.saturation, color.value)));
                strip.show();
                if (elapsedTime >= holdTime) {
                    state=Fade;
                    stateStart=millis();
                }                
                break;
            }
        }
    }

    void reset(){
        currentIndex = 0;
        nextIndex = 1;
        state=Hold;
        stateStart = millis();
    }

    private:
    float sinTimeMapper(float time) {
        return 0.5+0.5*sin(PI*time-PI/2);

        // Step time mapper:
        // if (time >=0.5) {
        //     return 1;
        // } else {
        //     return 0;
        // }
    }

    void incrementList(){
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
    }

    void fadeLEDs(){
        HSVColor startColor = colorList[currentIndex];
        HSVColor targetColor = colorList[nextIndex];
        float time = sinTimeMapper(elapsedTime/fadeTime);
        // float time = elapsedTime/fadeTime;
        Serial.println(time);
        HSVColor color = gradient(startColor, targetColor, time);
        strip.fill(strip.gamma32(strip.ColorHSV(color.hue, color.saturation, color.value)));
        strip.show();
    }

    enum State {
        Fade,
        Hold
    };
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

    LedFader::State state;
    int currentIndex;
    int nextIndex;
    unsigned long stateStart;
    unsigned long elapsedTime;
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

HSVColor fadeCyan[2] = {{32768, 255, 30}, {32768, 255, 255}};
HSVColor fadeMagenta[2] = {{54613, 255, 30}, {54613, 255, 255}};
HSVColor multiColorFade[7] = {{REDHUE, 255, 150}, 
                              {ORANGEHUE, 255, 150}, 
                              {YELLOWHUE, 255, 150}, 
                              {GREENHUE, 255, 150}, 
                              {CYANHUE, 255, 150}, 
                              {BLUEHUE, 255, 150}, 
                              {MAGENTAHUE, 255, 150}};

LedFader cyanFader(fadeCyan,2, 2000, 0);
LedFader magentaFader(fadeMagenta,2, 2000, 0);
LedFader multiColorFader(multiColorFade, 7, 2000, 3000);

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
    magentaFader.update();
    break;
  case FADE_CYAN:
    cyanFader.update();
    break;
  case FADE_MULTI:
    multiColorFader.update();
    break;
  default:
    break;
  }

}