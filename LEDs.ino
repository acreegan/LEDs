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
#define OFF 0x19


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);

unsigned long previousMillis = 0;
int brightness = 1;
int fadeDirection = 1;

uint16_t command;

class LedFader {
    public:
    int hue;
    int saturation;
    int delayInterval;
    float fadeTime;
    LedFader(int hue = 0, int saturation = 255, int fadeTime = 2550, int delayInterval=10) 
             :hue(hue), saturation(saturation), fadeTime(fadeTime), delayInterval(delayInterval) {}

    void fadeLEDs(){
        if (intervalDelay()) {
            unsigned long elapsedTime = millis() - fadeStart;
            int sourceBrightness = brightnessList[currentBrightnessIndex];
            int targetBrightness = brightnessList[nextBrightnessIndex];
            int brightness = gradient(sourceBrightness, targetBrightness, elapsedTime/fadeTime);
            strip.fill(strip.gamma32(strip.ColorHSV(hue, saturation, brightness)));
            strip.show();

            if (elapsedTime >= fadeTime) {
                currentBrightnessIndex = nextBrightnessIndex;
                nextBrightnessIndex = (nextBrightnessIndex+1) % (sizeof(brightnessList)/sizeof(brightnessList[0]));
                fadeStart = millis();
            }
        }
    }

    void reset(){
        currentBrightnessIndex = 0;
        nextBrightnessIndex = 1;
        fadeStart = millis();
    }

    private:
    // Calculate a point between two brightnesses
    int gradient(int sourceBrightness, int targetBrightness, float proportion) {
        return sourceBrightness + int((targetBrightness-sourceBrightness)*proportion);
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
    int brightnessList[2] = {1, 255};
    int currentBrightnessIndex = 0;
    int nextBrightnessIndex = 1;
    unsigned long fadeStart = 0;
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

LedFader cyanFader(32768);
LedFader magentaFader(54613);

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
  default:
    break;
  }

}