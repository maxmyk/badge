# Animated Badge
Weekend project based on ESP32-S2 mini, 2" TFT ST7789 240x320 RGB display, and some janky soldering. It took precisely 2 evenings to make.

![Animated demo](media/badge_demo_s.webp)

That's a badge, not just a display, because it has some room between the display and other hardware. So, you can wear it in/on your shirt's pocket.

![Space between the display and the rest of the components](media/image1.png)

Also, it features USB-C charging and 1 hour of autonomy.

## Dependencies
- [Adafruit_ST7789 library](https://github.com/adafruit/Adafruit-ST7735-Library/tree/master)
- [QR Code generator library](https://www.nayuki.io/page/qr-code-generator-library)
- [Image to C++ Byte Array Converter](https://image2cpp.42web.io/)

## How to run
As with every ESP32 project with the Arduino core, install https://github.com/espressif/arduino-esp32 via the boards manager. Arduino core lowers the entry point for most people. Arduino == easy(ier).

Open `badge.ino` using Arduino IDE

Change the name, description, and image (`photo.h`) to yours.

Select LOLIN32-S2 mini or your board if you use a different one (don't forget to change the TFT_MOSI and TFT_SCLK accordingly)

Wire/solder everything correctly. Double-check SCL and SDA connections.

[For ESP32-S2 mini] hold BOOT (0), press&release RESET (RST), then release BOOT

Upload the code

Click the BOOT (0) button to cycle through the screens

Hold the BOOT (0) button for ~1 second to switch between auto and manual modes


## Story
I got myself a small display I've been wanting to experiment with for a long time. The idea occurred spontaneously.

First, I created a breadboard prototype and played with the display for a little while.

Then, I decided to make it more rigid by using a prototyping board. I created a PCB design in Fritzing because it's really easy to use for quick prototyping, and it has a large number of parts readily available.

![PCB design](media/display_pcb.png)

I used a soldering iron to trace the board and made a rigid connection between components.

Also, I made the prototype modular - each part can be quickly disconnected and swapped.

This prototype includes a 200 mAh Li-Ion battery and a BMS board. I used a hot glue gun to fix them in place. Based on 7+ tests, it can display the animation for ~1 hour. The biggest power draw comes from the display. I may use a different type of display in the future, most likely an E-Ink one.

![Prototype, no display](media/image.png)

The power can be disconnected using an on-board switch. Also, the board features an additional safety feature. The ESP module can't be programmed without physically disconnecting the battery - connecting pins are blocking the USB port.

![On-board power switch](media/image2.png)

I may not have a 3D printer or fancy equipment, but I do have free will, programming knowledge, a soldering iron, lots of electrical tape, a hot glue gun, and many ideas.

## Afterword

There was a reliability issue. At first, I thought that there was a short somewhere on the hand-soldered perfboard. The next suspect was the SPI connection. Then I thought the display was drawing too much power, so I soldered a 100uF capacitor onto the power line. It seemed to work for a bit, but it wasn't stable. After poking around with the tester, I found that the power line for the display (even though thoroughly soldered) wasn't reliable. I had to re-solder it. After that, everything worked fine.

After using it at a few events, I found the current version too bulky and, at the same time, too small. Power consumption is also a problem. I may create a new version with a bigger battery and a more efficient & larger display. The bulkiness comes from the modular design, which is not necessary for a badge. I may also make a custom PCB for the next version. Also, the initial design with auto screen switching was not that convenient, so I added a manual mode. Now, when someone asks to see the QR code, I can just click the button a few times to show it, instead of waiting ~30 seconds for the auto mode to cycle through the screens.