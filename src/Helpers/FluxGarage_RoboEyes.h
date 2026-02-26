#ifndef _FLUXGARAGE_ROBOEYES_H
#define _FLUXGARAGE_ROBOEYES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>

// Display colors
static uint8_t BGCOLOR = 0;
static uint8_t MAINCOLOR = 1;

// For mood type switch - biểu cảm
#define DEFAULT   0
#define TIRED     1
#define ANGRY     2
#define HAPPY     3
#define SLEEP     4

// For turning things on or off
#define ON  1
#define OFF 0

// For switch "predefined positions" - vị trí mắt
#define N   1 // north, top center
#define NE  2 // north-east, top right
#define E   3 // east, middle right
#define SE  4 // south-east, bottom right
#define S   5 // south, bottom center
#define SW  6 // south-west, bottom left
#define W   7 // west, middle left
#define NW  8 // north-west, top left 
// for middle center set "DEFAULT"

template<typename AdafruitDisplay>
class RoboEyes
{
  public:
    AdafruitDisplay *display;
    bool isOLED = false; // true: dùng OLED, false: dùng TFT

    int screenWidth = 128;
    int screenHeight = 64;
    int frameInterval = 20;
    unsigned long fpsTimer = 0;

    bool tired = 0;
    bool angry = 0;
    bool happy = 0;
    bool curious = 0;
    bool cyclops = 0;
    bool eyeL_open = 0;
    bool eyeR_open = 0;

    // Space between eyes (đưa lên đầu để các biến khác dùng)
    int spaceBetweenDefault = 10;
    int spaceBetweenCurrent = spaceBetweenDefault;
    int spaceBetweenNext = 10;

    // EYE LEFT
    int eyeLwidthDefault = 36;
    int eyeLheightDefault = 36;
    int eyeLwidthCurrent = eyeLwidthDefault;
    int eyeLheightCurrent = 1;
    int eyeLwidthNext = eyeLwidthDefault;
    int eyeLheightNext = eyeLheightDefault;
    int eyeLheightOffset = 0;
    byte eyeLborderRadiusDefault = 8;
    byte eyeLborderRadiusCurrent = eyeLborderRadiusDefault;
    byte eyeLborderRadiusNext = eyeLborderRadiusDefault;

    // EYE RIGHT
    int eyeRwidthDefault = eyeLwidthDefault;
    int eyeRheightDefault = eyeLheightDefault;
    int eyeRwidthCurrent = eyeRwidthDefault;
    int eyeRheightCurrent = 1;
    int eyeRwidthNext = eyeRwidthDefault;
    int eyeRheightNext = eyeRheightDefault;
    int eyeRheightOffset = 0;
    byte eyeRborderRadiusDefault = 8;
    byte eyeRborderRadiusCurrent = eyeRborderRadiusDefault;
    byte eyeRborderRadiusNext = eyeRborderRadiusDefault;

    // EYE LEFT - Coordinates
    int eyeLxDefault = ((screenWidth)-(eyeLwidthDefault+spaceBetweenDefault+eyeRwidthDefault))/2;
    int eyeLyDefault = ((screenHeight-eyeLheightDefault)/2);
    int eyeLx = eyeLxDefault;
    int eyeLy = eyeLyDefault;
    int eyeLxNext = eyeLx;
    int eyeLyNext = eyeLy;

    // EYE RIGHT - Coordinates
    int eyeRxDefault = eyeLx+eyeLwidthCurrent+spaceBetweenDefault;
    int eyeRyDefault = eyeLy;
    int eyeRx = eyeRxDefault;
    int eyeRy = eyeRyDefault;
    int eyeRxNext = eyeRx;
    int eyeRyNext = eyeRy;

    // Eyelid top size
    byte eyelidsHeightMax = eyeLheightDefault/2;
    byte eyelidsTiredHeight = 0;
    byte eyelidsTiredHeightNext = eyelidsTiredHeight;
    byte eyelidsAngryHeight = 0;
    byte eyelidsAngryHeightNext = eyelidsAngryHeight;
    byte eyelidsHappyBottomOffsetMax = (eyeLheightDefault/2)+3;
    byte eyelidsHappyBottomOffset = 0;
    byte eyelidsHappyBottomOffsetNext = 0;

    // Macro Animations
    bool hFlicker = 0;
    bool hFlickerAlternate = 0;
    byte hFlickerAmplitude = 2;
    bool vFlicker = 0;
    bool vFlickerAlternate = 0;
    byte vFlickerAmplitude = 10;
    bool autoblinker = 0;
    int blinkInterval = 1;
    int blinkIntervalVariation = 4;
    unsigned long blinktimer = 0;
    bool idle = 0;
    int idleInterval = 1;
    int idleIntervalVariation = 3;
    unsigned long idleAnimationTimer = 0;
    bool confused = 0;
    unsigned long confusedAnimationTimer = 0;
    int confusedAnimationDuration = 500;
    bool confusedToggle = 1;
    bool laugh = 0;
    unsigned long laughAnimationTimer = 0;
    int laughAnimationDuration = 500;
    bool laughToggle = 1;
    bool sleeping = 0;
    unsigned long sleepTimer = 0;
    int sleepInterval = 500;
    bool sleepToggle = 0;
    float bubbleX = 0;
    float bubbleY = 0;
    bool bubbleActive = 0;
    unsigned long bubbleTimer = 0;
    int bubbleInterval = 3000;
    bool sweat = 0;
    byte sweatBorderradius = 3;

    // Sweat drop 1
    int sweat1XPosInitial = 2;
    int sweat1XPos = 2;
    float sweat1YPos = 2;
    int sweat1YPosMax = 10;
    float sweat1Height = 2;
    float sweat1Width = 1;

    // Sweat drop 2
    int sweat2XPosInitial = 2;
    int sweat2XPos = 2;
    float sweat2YPos = 2;
    int sweat2YPosMax = 10;
    float sweat2Height = 2;
    float sweat2Width = 1;

    // Sweat drop 3
    int sweat3XPosInitial = 2;
    int sweat3XPos = 2;
    float sweat3YPos = 2;
    int sweat3YPosMax = 10;
    float sweat3Height = 2;
    float sweat3Width = 1;


  //*********************************************************************************************
  //  GENERAL METHODS
  //*********************************************************************************************

  RoboEyes(AdafruitDisplay &disp, bool oled = true) : display(&disp), isOLED(oled) {};

  // Startup RoboEyes with defined screen-width, screen-height and max. frames per second
  void begin(int width, int height, byte frameRate) {
    screenWidth = width;
    screenHeight = height;
    if (isOLED) {
      // display->clearDisplay();
      // display->display();
    } 
    else {
      display->fillScreen(BGCOLOR);
    }
    eyeLheightCurrent = 1;
    eyeRheightCurrent = 1;
    setFramerate(frameRate);
  }

  void update(){
    // Limit drawing updates to defined max framerate
    if(millis()-fpsTimer >= frameInterval){
      drawEyes();
      fpsTimer = millis();
    }
  }


  //*********************************************************************************************
  //  SETTERS METHODS
  //*********************************************************************************************

  // Calculate frame interval based on defined frameRate
  void setFramerate(byte fps){
    frameInterval = 1000/fps;
  }

  // Set color values
  void setDisplayColors(uint8_t background, uint8_t main) {
    BGCOLOR = background; // background and overlays, choose 0 for monochrome displays and 0x00 for grayscale displays such as SSD1322
    MAINCOLOR = main; // drawings, choose 1 for monochrome displays and 0x0F for grayscale displays such as SSD1322 (0x0F = maximum brightness)
  }

  void setWidth(byte leftEye, byte rightEye) {
    eyeLwidthNext = leftEye;
    eyeRwidthNext = rightEye;
    eyeLwidthDefault = leftEye;
    eyeRwidthDefault = rightEye;
  }

  void setHeight(byte leftEye, byte rightEye) {
    eyeLheightNext = leftEye;
    eyeRheightNext = rightEye;
    eyeLheightDefault = leftEye;
    eyeRheightDefault = rightEye;
  }

  // Set border radius for left and right eye
  void setBorderradius(byte leftEye, byte rightEye) {
    eyeLborderRadiusNext = leftEye;
    eyeRborderRadiusNext = rightEye;
    eyeLborderRadiusDefault = leftEye;
    eyeRborderRadiusDefault = rightEye;
  }

  // Set space between the eyes, can also be negative
  void setSpacebetween(int space) {
    spaceBetweenNext = space;
    spaceBetweenDefault = space;
  }

  // Set mood expression
  void setMood(unsigned char mood) {
    switch (mood) {
      case TIRED:
        tired=1; 
        angry=0; 
        happy=0;
        sleeping = 0;
        break;
      case ANGRY:
        tired=0; 
        angry=1; 
        happy=0;
        sleeping = 0;
        break;
      case HAPPY:
        tired=0; 
        angry=0; 
        happy=1;
        sleeping = 0;
        break;
      case SLEEP:
        tired = 0;
        angry = 0;
        happy = 0;
        sleeping = 1;   
        idle = 0;             // tắt idle mode
        autoblinker = 0;      // tắt blink tự động
        eyeLxNext = screenWidth/2 - eyeLwidthDefault/2; // mắt trái ở giữa
        eyeLyNext = screenHeight/2 - eyeLheightDefault/2;   
        close();           
        break;
      default:
        tired=0; 
        angry=0; 
        happy=0;
        sleeping = 0;
        idle = 1;           
        autoblinker = 1;
        break;
    }
  }

  // Set predefined position
  void setPosition(unsigned char position)
    {
      switch (position)
      {
      case N:
        // North, top center
        eyeLxNext = getScreenConstraint_X()/2;
        eyeLyNext = 0;
        break;
      case NE:
        // North-east, top right
        eyeLxNext = getScreenConstraint_X();
        eyeLyNext = 0;
        break;
      case E:
        // East, middle right
        eyeLxNext = getScreenConstraint_X();
        eyeLyNext = getScreenConstraint_Y()/2;
        break;
      case SE:
        // South-east, bottom right
        eyeLxNext = getScreenConstraint_X();
        eyeLyNext = getScreenConstraint_Y();
        break;
      case S:
        // South, bottom center
        eyeLxNext = getScreenConstraint_X()/2;
        eyeLyNext = getScreenConstraint_Y();
        break;
      case SW:
        // South-west, bottom left
        eyeLxNext = 0;
        eyeLyNext = getScreenConstraint_Y();
        break;
      case W:
        // West, middle left
        eyeLxNext = 0;
        eyeLyNext = getScreenConstraint_Y()/2;
        break;
      case NW:
        // North-west, top left
        eyeLxNext = 0;
        eyeLyNext = 0;
        break;
      default:
        // Middle center
        eyeLxNext = getScreenConstraint_X()/2;
        eyeLyNext = getScreenConstraint_Y()/2;
        break;
      }
    }

  // Set automated eye blinking, minimal blink interval in full seconds and blink interval variation range in full seconds
  void setAutoblinker(bool active, int interval, int variation){
    autoblinker = active;
    blinkInterval = interval;
    blinkIntervalVariation = variation;
  }
  void setAutoblinker(bool active){
    autoblinker = active;
  }

  // Set idle mode - automated eye repositioning, minimal time interval in full seconds and time interval variation range in full seconds
  void setIdleMode(bool active, int interval, int variation){
    idle = active;
    idleInterval = interval;
    idleIntervalVariation = variation;
  }
  void setIdleMode(bool active) {
    idle = active;
  }

  // Set curious mode - the respectively outer eye gets larger when looking left or right
  void setCuriosity(bool curiousBit) {
    curious = curiousBit;
  }

  // Set cyclops mode - show only one eye 
  void setCyclops(bool cyclopsBit) {
    cyclops = cyclopsBit;
  }

  // Set horizontal flickering (displacing eyes left/right)
  void setHFlicker (bool flickerBit, byte Amplitude) {
    hFlicker = flickerBit; // turn flicker on or off
    hFlickerAmplitude = Amplitude; // define amplitude of flickering in pixels
  }
  void setHFlicker (bool flickerBit) {
    hFlicker = flickerBit; // turn flicker on or off
  }

  // Set vertical flickering (displacing eyes up/down)
  void setVFlicker (bool flickerBit, byte Amplitude) {
    vFlicker = flickerBit; // turn flicker on or off
    vFlickerAmplitude = Amplitude; // define amplitude of flickering in pixels
  }
  void setVFlicker (bool flickerBit) {
    vFlicker = flickerBit; // turn flicker on or off
  }

  void setSweat (bool sweatBit) {
    sweat = sweatBit; // turn sweat on or off
  }


  //*********************************************************************************************
  //  GETTERS METHODS
  //*********************************************************************************************

  // Returns the max x position for left eye
  int getScreenConstraint_X(){
    return screenWidth-eyeLwidthCurrent-spaceBetweenCurrent-eyeRwidthCurrent;
  } 

  // Returns the max y position for left eye
  int getScreenConstraint_Y(){
    return screenHeight-eyeLheightDefault; // using default height here, because height will vary when blinking and in curious mode
  }


  //*********************************************************************************************
  //  BASIC ANIMATION METHODS
  //*********************************************************************************************

  // BLINKING FOR BOTH EYES AT ONCE
  // Close both eyes
  void close() {
    eyeLheightNext = 1; // closing left eye
    eyeRheightNext = 1; // closing right eye
    eyeL_open = 0; // left eye not opened (=closed)
    eyeR_open = 0; // right eye not opened (=closed)
  }

  // Open both eyes
  void open() {
    eyeL_open = 1; // left eye opened - if true, drawEyes() will take care of opening eyes again
    eyeR_open = 1; // right eye opened
  }

  // Trigger eyeblink animation
  void blink() {
    close();
    open();
  }

  // BLINKING FOR SINGLE EYES, CONTROL EACH EYE SEPARATELY
  // Close eye(s)
  void close(bool left, bool right) {
    if(left){
      eyeLheightNext = 1; // blinking left eye
      eyeL_open = 0; // left eye not opened (=closed)
    }
    if(right){
        eyeRheightNext = 1; // blinking right eye
        eyeR_open = 0; // right eye not opened (=closed)
    }
  }

  // Open eye(s)
  void open(bool left, bool right) {
    if(left){
      eyeL_open = 1; // left eye opened - if true, drawEyes() will take care of opening eyes again
    }
    if(right){
      eyeR_open = 1; // right eye opened
    }
  }

  // Trigger eyeblink(s) animation
  void blink(bool left, bool right) {
    close(left, right);
    open(left, right);
  }


  //*********************************************************************************************
  //  MACRO ANIMATION METHODS
  //*********************************************************************************************

  // Play confused animation - one shot animation of eyes shaking left and right
  void anim_confused() {
    confused = 1;
  }

  // Play laugh animation - one shot animation of eyes shaking up and down
  void anim_laugh() {
    laugh = 1;
  }

  //*********************************************************************************************
  //  PRE-CALCULATIONS AND ACTUAL DRAWINGS
  //*********************************************************************************************

  void drawEyes(){

    //// PRE-CALCULATIONS - EYE SIZES AND VALUES FOR ANIMATION TWEENINGS ////

    // Vertical size offset for larger eyes when looking left or right (curious gaze)
    if(curious){
      if(eyeLxNext<=10){eyeLheightOffset=8;}
      else if (eyeLxNext>=(getScreenConstraint_X()-10) && cyclops){eyeLheightOffset=8;}
      else{eyeLheightOffset=0;} // left eye
      if(eyeRxNext>=screenWidth-eyeRwidthCurrent-10){eyeRheightOffset=8;}
      else{eyeRheightOffset=0;} // right eye
    } 
    else {
      eyeLheightOffset=0; // reset height offset for left eye
      eyeRheightOffset=0; // reset height offset for right eye
    }

    // Left eye height
    eyeLheightCurrent = (eyeLheightCurrent + eyeLheightNext + eyeLheightOffset)/2;
    eyeLy+= ((eyeLheightDefault-eyeLheightCurrent)/2); // vertical centering of eye when closing
    eyeLy-= eyeLheightOffset/2;
    // Right eye height
    eyeRheightCurrent = (eyeRheightCurrent + eyeRheightNext + eyeRheightOffset)/2;
    eyeRy+= (eyeRheightDefault-eyeRheightCurrent)/2; // vertical centering of eye when closing
    eyeRy-= eyeRheightOffset/2;


    // Open eyes again after closing them
    if(eyeL_open){
      if(eyeLheightCurrent <= 1 + eyeLheightOffset){eyeLheightNext = eyeLheightDefault;} 
    }
    if(eyeR_open){
      if(eyeRheightCurrent <= 1 + eyeRheightOffset){eyeRheightNext = eyeRheightDefault;} 
    }

    // Left eye width
    eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthNext)/2;
    // Right eye width
    eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthNext)/2;


    // Space between eyes
    spaceBetweenCurrent = (spaceBetweenCurrent + spaceBetweenNext)/2;

    // Left eye coordinates
    eyeLx = (eyeLx + eyeLxNext)/2;
    eyeLy = (eyeLy + eyeLyNext)/2;
    // Right eye coordinates
    eyeRxNext = eyeLxNext+eyeLwidthCurrent+spaceBetweenCurrent; // right eye's x position depends on left eyes position + the space between
    eyeRyNext = eyeLyNext; // right eye's y position should be the same as for the left eye
    eyeRx = (eyeRx + eyeRxNext)/2;
    eyeRy = (eyeRy + eyeRyNext)/2;

    // Left eye border radius
    eyeLborderRadiusCurrent = (eyeLborderRadiusCurrent + eyeLborderRadiusNext)/2;
    // Right eye border radius
    eyeRborderRadiusCurrent = (eyeRborderRadiusCurrent + eyeRborderRadiusNext)/2;
    

    //// APPLYING MACRO ANIMATIONS ////

    if(autoblinker){
      if(millis() >= blinktimer){
      blink();
      blinktimer = millis()+(blinkInterval*1000)+(random(blinkIntervalVariation)*1000); // calculate next time for blinking
      }
    }

    // Laughing - eyes shaking up and down for the duration defined by laughAnimationDuration (default = 500ms)
    if(laugh){
      if(laughToggle){
        setVFlicker(1, 5);
        laughAnimationTimer = millis();
        laughToggle = 0;
      } 
      else if(millis() >= laughAnimationTimer+laughAnimationDuration){
        setVFlicker(0, 0);
        laughToggle = 1;
        laugh=0; 
      }
    }

    // Confused - eyes shaking left and right for the duration defined by confusedAnimationDuration (default = 500ms)
    if(confused){
      if(confusedToggle){
        setHFlicker(1, 20);
        confusedAnimationTimer = millis();
        confusedToggle = 0;
      } 
      else if(millis() >= confusedAnimationTimer+confusedAnimationDuration){
        setHFlicker(0, 0);
        confusedToggle = 1;
        confused=0; 
      }
    }

    // Idle - eyes moving to random positions on screen
    if(idle){
      if(millis() >= idleAnimationTimer){
        eyeLxNext = random(getScreenConstraint_X());
        eyeLyNext = random(getScreenConstraint_Y());
        idleAnimationTimer = millis()+(idleInterval*1000)+(random(idleIntervalVariation)*1000); // calculate next time for eyes repositioning
      }
    }

    // Adding offsets for horizontal flickering/shivering
    if(hFlicker){
      if(hFlickerAlternate) {
        eyeLx += hFlickerAmplitude;
        eyeRx += hFlickerAmplitude;
      } 
      else {
        eyeLx -= hFlickerAmplitude;
        eyeRx -= hFlickerAmplitude;
      }
      hFlickerAlternate = !hFlickerAlternate;
    }

    // Adding offsets for horizontal flickering/shivering
    if(vFlicker){
      if(vFlickerAlternate) {
        eyeLy += vFlickerAmplitude;
        eyeRy += vFlickerAmplitude;
      } 
      else {
        eyeLy -= vFlickerAmplitude;
        eyeRy -= vFlickerAmplitude;
      }
      vFlickerAlternate = !vFlickerAlternate;
    }

    // Cyclops mode, set second eye's size and space between to 0
    if(cyclops){
      eyeRwidthCurrent = 0;
      eyeRheightCurrent = 0;
      spaceBetweenCurrent = 0;
    }

    //// ACTUAL DRAWINGS ////

    if (isOLED) {
      // display->clearDisplay(); // start with a blank screen
    } else {
      display->fillScreen(BGCOLOR);
    }

    // Draw basic eye rectangles
    display->fillRoundRect(eyeLx, eyeLy, eyeLwidthCurrent, eyeLheightCurrent, eyeLborderRadiusCurrent, MAINCOLOR); // left eye
    if (!cyclops){
      display->fillRoundRect(eyeRx, eyeRy, eyeRwidthCurrent, eyeRheightCurrent, eyeRborderRadiusCurrent, MAINCOLOR); // right eye
    }

    // Prepare mood type transitions
    if (tired){eyelidsTiredHeightNext = eyeLheightCurrent/2; eyelidsAngryHeightNext = 0;} else{eyelidsTiredHeightNext = 0;}
    if (angry){eyelidsAngryHeightNext = eyeLheightCurrent/2; eyelidsTiredHeightNext = 0;} else{eyelidsAngryHeightNext = 0;}
    if (happy){eyelidsHappyBottomOffsetNext = eyeLheightCurrent/2;} else{eyelidsHappyBottomOffsetNext = 0;}

    // Draw tired top eyelids 
    eyelidsTiredHeight = (eyelidsTiredHeight + eyelidsTiredHeightNext)/2;
    if (!cyclops){
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // left eye 
      display->fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy+eyelidsTiredHeight-1, BGCOLOR); // right eye
    } else {
      // Cyclops tired eyelids
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // left eyelid half
      display->fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // right eyelid half
    }

    // Draw angry top eyelids 
    eyelidsAngryHeight = (eyelidsAngryHeight + eyelidsAngryHeightNext)/2;
    if (!cyclops){ 
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsAngryHeight-1, BGCOLOR); // left eye
      display->fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx, eyeRy+eyelidsAngryHeight-1, BGCOLOR); // right eye
    } else {
      // Cyclops angry eyelids
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR); // left eyelid half
      display->fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR); // right eyelid half
    }

    // Draw happy bottom eyelids
    eyelidsHappyBottomOffset = (eyelidsHappyBottomOffset + eyelidsHappyBottomOffsetNext)/2;
    display->fillRoundRect(eyeLx-1, (eyeLy+eyeLheightCurrent)-eyelidsHappyBottomOffset+1, eyeLwidthCurrent+2, eyeLheightDefault, eyeLborderRadiusCurrent, BGCOLOR); // left eye
    if (!cyclops){ 
      display->fillRoundRect(eyeRx-1, (eyeRy+eyeRheightCurrent)-eyelidsHappyBottomOffset+1, eyeRwidthCurrent+2, eyeRheightDefault, eyeRborderRadiusCurrent, BGCOLOR); // right eye
    }

    // Add sweat drops
    if (sweat){
      // Sweat drop 1 -> left corner
      if(sweat1YPos <= sweat1YPosMax){
        sweat1YPos+=0.5;
      } // vertical movement from initial to max
      else {
        sweat1XPosInitial = random(30); 
        sweat1YPos = 2; 
        sweat1YPosMax = (random(10)+10); 
        sweat1Width = 1; 
        sweat1Height = 2;
      } // if max vertical position is reached: reset all values for next drop
      if(sweat1YPos <= sweat1YPosMax/2){
        sweat1Width+=0.5; 
        sweat1Height+=0.5;
      } // shape grows in first half of animation ...
      else {
        sweat1Width-=0.1; 
        sweat1Height-=0.5;
      } // ... and shrinks in second half of animation
      sweat1XPos = sweat1XPosInitial-(sweat1Width/2); // keep the growing shape centered to initial x position
      display->fillRoundRect(sweat1XPos, sweat1YPos, sweat1Width, sweat1Height, sweatBorderradius, MAINCOLOR); // draw sweat drop


      // Sweat drop 2 -> center area
      if(sweat2YPos <= sweat2YPosMax){sweat2YPos+=0.5;} // vertical movement from initial to max
      else {sweat2XPosInitial = random((screenWidth-60))+30; sweat2YPos = 2; sweat2YPosMax = (random(10)+10); sweat2Width = 1; sweat2Height = 2;} // if max vertical position is reached: reset all values for next drop
      if(sweat2YPos <= sweat2YPosMax/2){sweat2Width+=0.5; sweat2Height+=0.5;} // shape grows in first half of animation ...
      else {sweat2Width-=0.1; sweat2Height-=0.5;} // ... and shrinks in second half of animation
      sweat2XPos = sweat2XPosInitial-(sweat2Width/2); // keep the growing shape centered to initial x position
      display->fillRoundRect(sweat2XPos, sweat2YPos, sweat2Width, sweat2Height, sweatBorderradius, MAINCOLOR); // draw sweat drop


      // Sweat drop 3 -> right corner
      if(sweat3YPos <= sweat3YPosMax){sweat3YPos+=0.5;} // vertical movement from initial to max
      else {sweat3XPosInitial = (screenWidth-30)+(random(30)); sweat3YPos = 2; sweat3YPosMax = (random(10)+10); sweat3Width = 1; sweat3Height = 2;} // if max vertical position is reached: reset all values for next drop
      if(sweat3YPos <= sweat3YPosMax/2){sweat3Width+=0.5; sweat3Height+=0.5;} // shape grows in first half of animation ...
      else {sweat3Width-=0.1; sweat3Height-=0.5;} // ... and shrinks in second half of animation
      sweat3XPos = sweat3XPosInitial-(sweat3Width/2); // keep the growing shape centered to initial x position
      display->fillRoundRect(sweat3XPos, sweat3YPos, sweat3Width, sweat3Height, sweatBorderradius, MAINCOLOR); // draw sweat drop
    }

    // Sleep animation - mắt phập phồng khi ngủ
    if (sleeping) {
      // Mắt phập phồng
      if (millis() >= sleepTimer + sleepInterval) {
        sleepTimer = millis();
        sleepToggle = !sleepToggle;
        if (sleepToggle) {
          eyeLheightNext = 3; // hé nhẹ
          eyeRheightNext = 3;
        } else {
          eyeLheightNext = 1; // nhắm sát
          eyeRheightNext = 1;
        }
      }

      // Bubble Zzz
      if (!bubbleActive && millis() >= bubbleTimer + bubbleInterval) {
        bubbleActive = 1;
        bubbleX = screenWidth/2 + 20;  // bắt đầu bên phải mắt
        bubbleY = screenHeight/2;      // cùng cao với mắt
        bubbleTimer = millis();
      }

      if (bubbleActive) {
        bubbleY -= 0.5;   // bong bóng bay lên
        if (bubbleY < 0) {
          bubbleActive = 0; // reset khi bay hết màn hình
        }
      }
    }

    // Draw bubble "Zzz" nếu đang ở sleep
    if (sleeping && bubbleActive) {
      display->drawCircle(bubbleX, bubbleY, 6, MAINCOLOR);   // vẽ bong bóng
      display->setTextSize(1);
      display->setTextColor(MAINCOLOR);
      display->setCursor(bubbleX - 3, bubbleY - 3);
      display->print("Z");
    }

    #ifdef isOLED
      display->display(); // show drawings on display
    #endif
    // TFT không cần gọi display()
  } // end of drawEyes method

}; // end of class roboEyes

#endif
