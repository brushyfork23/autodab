// Required Libraries:
// Bounce2 - Version: Latest - https://github.com/thomasfredericks/Bounce2
// Chrono - Version: Latest - https://github.com/SofaPirate/Chrono


//////////////////////////////////////////////////
// Manipulate these values to play with the timing
// and pitch of the three tones in the jingle
//////////////////////////////////////////////////
#define FIRST_TONE_PITCH    1250
#define FIRST_TONE_MILLIS   90
#define FIRST_PAUSE_MILLIS  20 + FIRST_TONE_MILLIS
#define SECOND_TONE_PITCH   1000
#define SECOND_TONE_MILLIS  100 + FIRST_PAUSE_MILLIS
#define SECOND_PAUSE_MILLIS 20 + SECOND_TONE_MILLIS
#define THIRD_TONE_PITCH    2000
#define THRID_TONE_MILLIS   150 + SECOND_PAUSE_MILLIS



// Timer conifg
#include <Chrono.h>
Chrono jingleTimer;

// Button config
#include <Bounce2.h>
#define PLAY_BTN_PIN    9
Bounce playBtn = Bounce();

// Buzzer config
#define BUZZER_PIN 10

bool buzzing = false;

void setup() {
  delay(100UL); // Give everything a moment to power up

  // Setup Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Setup Button
  playBtn.attach(PLAY_BTN_PIN, INPUT_PULLUP);
}

void loop() {
  // When button is pressed, rewind the jingle
  playBtn.update();
  if (playBtn.fell()) {
    resetJingle();
  }

  playJingle();
}

void playJingle() {
  unsigned long elapsed = jingleTimer.elapsed();
  if (!buzzing && elapsed < FIRST_TONE_MILLIS) {
    tone(BUZZER_PIN, FIRST_TONE_PITCH);
    buzzing = true;
  } else if (buzzing && elapsed >= FIRST_TONE_MILLIS) {
    noTone(BUZZER_PIN);
    buzzing = false;
  } else if (!buzzing && elapsed >= FIRST_PAUSE_MILLIS) {
    tone(BUZZER_PIN, SECOND_TONE_PITCH);
    buzzing = true;
  } else if (buzzing && elapsed >= SECOND_TONE_MILLIS) {
    noTone(BUZZER_PIN);
    buzzing = false;
  } else if (!buzzing && elapsed >= SECOND_PAUSE_MILLIS) {
    tone(BUZZER_PIN, THIRD_TONE_PITCH);
    buzzing = true;
  } else if (buzzing && elapsed >= THRID_TONE_MILLIS) {
    noTone(BUZZER_PIN);
    buzzing = false;
  }
}

void resetJingle() {
  noTone(BUZZER_PIN);
  buzzing = false;
  jingleTimer.restart();
}