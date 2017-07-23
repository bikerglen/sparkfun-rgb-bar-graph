/*

DPMOD   BGRAPH   T3.2       T3.2   BGRAPH   DPMOS
PIN #   SIGNAL   PIN        PIN    SIGNAL   PIN #
-----   ------   ----       ----   ------   -----
J3-5    GND      GND        VIN
                 0          AGND
                 1          3.3V   +3.3     J3-6
J3-9    DAT      2          23     A3       J3-8
J3-10   LAT*     3          22     A2       J3-2
J3-     OFF      4          21
                 5          20
                 6          19
                 7          18
J3-10   LAT*     8          17
J3-1    A0       9          16
J3-7    A1       10         15
                 11         14     CLK      J3-4
                 12         13

DPMOD PIN #   = PMOD Pin # using Digilent's pin numbering scheme instead of normal header pin numbering scheme
BGRAPH SIGNAL = Bargraph driver signal name from board silkscreen
T3.2 PIN      = Teensy 3.2 board pin name / number

*/

#include <SmartMatrix3.h>
#include "inttypes.h"
#include "perlin.h"

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 32;       // known working: 32, 64, 96, 128
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

int led = 13;

Perlin perlin;
uint8_t levels[144]; // 1 row * 48 columns * 3 colors, segment 0 {R,G,B} ... segment 47 {R,G,B}

void setup() {
  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(255);
  pinMode(led, OUTPUT);
  perlin.init ();
}

void loop() {
  digitalWrite(led, HIGH);
  backgroundLayer.fillScreen({0, 0, 0});
  perlin.next (levels);
  uint8_t *plevels = levels;
  levels[0] = 0;
  levels[1] = 0;
  levels[2] = 0;
  for (int n = 0; n < 48; n++) {
    uint8_t row = n / 3;
    uint8_t col = 29 - 3 * (n % 3);
    SM_RGB pixel = { 0, 0 , 0 };
    pixel.red = *plevels++;
    backgroundLayer.drawPixel(col++, row, pixel);
    pixel.red = *plevels++;
    backgroundLayer.drawPixel(col++, row, pixel);
    pixel.red = *plevels++;
    backgroundLayer.drawPixel(col++, row, pixel);
  }
  backgroundLayer.swapBuffers();
  digitalWrite(led, LOW);
  delay(10);
}

