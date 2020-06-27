class RotaryEncoder {
  public:
    RotaryEncoder(uint8_t clk, uint8_t dt)
    :delta(0), clkPin(clk), dtPin(dt) {
      pinMode(clk, INPUT);
      pinMode(dt, INPUT);
    }

    void update() {
      // Get a new CLK reading
      bool currentClkState = digitalRead(clkPin);

      // If last and current state of CLK are different, then pulse occurred.
      // React to only 1 state change to avoid double count.
      if (currentClkState != lastClkState && currentClkState == HIGH) {
        // If the DT state is different than the CLK state then
        // the encoder is rotating CCW so decrement
        if (digitalRead(dtPin) != currentClkState) {
          delta++;
        } else {
          delta--;
        }
      }
      lastClkState = currentClkState; // Remember last CLK state
    }

    int16_t getChange() {
      int16_t val = delta;
      delta = 0;
      return val;
    }

    void reset() {
      delta = 0;
    }

  private:
    const uint8_t clkPin;
    const uint8_t dtPin;
    int16_t delta;
    bool lastClkState;
};