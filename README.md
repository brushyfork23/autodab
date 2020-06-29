# Autodab

This controller runs an automated dab rig, heating the nail and letting you know when it's ready to hit.

## Operation
When the **GO** button is pressed, the controller cycles through 3 states: Heating, Cooling, and Ready.  
1. Heating: A solenoid is actuated to light a torch which heats the dab rig's nail.  A ring of LEDs grually fills red.
2. Cooling: The solenoid is disengaged, and the nail cools down.  The LEDs gradually turn off as they fade from red to green.
3. Ready: The nail is at the perfect temperature to hit!  A jingle is played and all the LEDs light up green, then gradually fade as the nail continues to cool.

The timing of each stage can be changed by pressing the **MODE** button to cycle through them and using a rotary knob to adjust adjust.
1. First Press: Set heating time between 10s and 70s.
2. Second Press: Set cooling time between 1s and 60s.
3. Third Press: Set ready-to-hit time between 1s and 60s.

## Components
* [Adafruit Metro](https://www.adafruit.com/product/2488) (or probbaly any Arduino Uno compatible board).
* Solenoid
* strand of 60 WS2811 RGB LEDs
* Rotary encoder
* GO normally-open momentary button
* MODE normally-open momentary button
* Buzzer

## Wiring
| connect this  | to this            | 
| :-----------  | :----------        |
| Metro pin 3   | LED Data           |
| Metro pin 4   | Rotary Encoder CLK |
| Metro pin 5   | Rotary Encoder DT  |
| Metro pin 6   | MODE button +      |
| Metro pin 9   | GO button +        |
| Metro pin 7   | Solenoid +         |
| Metro pin 10  | Buzzer +           |
| LED +         | +5 VDC             |
| LED -         | GND                |
| MODE button - | GND                |
| GO button -   | GND                |
| Solenoid -    | GND                |
| Buzzer -      | GND                |