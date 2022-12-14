#include <Arduino.h>

#include <gfx.hpp>
#include <htcw_button.hpp>
#include <st7789.hpp>
#include <tft_io.hpp>
#include <fonts/Ubuntu.hpp>
#define LCD_WIDTH 135
#define LCD_HEIGHT 240
#define LCD_HOST VSPI
#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 19
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 16
#define PIN_NUM_RST 23
#define PIN_NUM_BCKL 4
using namespace arduino;
using namespace gfx;

using bus_t = tft_spi_ex<LCD_HOST, PIN_NUM_CS, PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, SPI_MODE0,true,LCD_WIDTH*LCD_HEIGHT*2+8,2>;
using display_t = st7789<LCD_WIDTH, LCD_HEIGHT, PIN_NUM_DC, PIN_NUM_RST, PIN_NUM_BCKL, bus_t, 0, true, 400, 200>;
using color_t = color<typename display_t::pixel_type>;

using button_1_t = button<35, 10, true>;
using button_2_t = button<0, 10, true>;

static display_t dsp;

button_1_t button_1;
button_2_t button_2;

// SNIP ---- delete after the above line for new projects.

const int16_t
    res_bits = 12,                        // Fractional resolution
    pixelWidth = dsp.dimensions().width,  // TFT dimensions
    pixelHeight = dsp.dimensions().height,
    iterations = 20;  // Fractal iteration limit or 'dwell'
float
    centerReal(-0.6),  // Image center point in complex plane
    centerImag(0.0),
    rangeReal(3.0),  // Image coverage in complex plane
    rangeImag(3.0),
    incRange(.95);

static void button_1_cb(bool pressed, void* state) {
    Serial.printf("Button 1 %s\n",pressed?"pressed":"released");
}
static void button_2_cb(bool pressed, void* state) {
    Serial.printf("Button 2 %s\n",pressed?"pressed":"released");
}
void setup() {
    Serial.begin(115200);
    button_1.initialize();
    button_2.initialize();
    button_1.callback(button_1_cb);
    button_2.callback(button_2_cb);
    draw::filled_rectangle(dsp, dsp.bounds(), color_t::black);

    open_text_info oti;
    oti.font = &Ubuntu;
    oti.text = "Mandelbrot";
    oti.transparent_background = false;
    // 25 pixel high font
    oti.scale = oti.font->scale(25);
    // center the text
    ssize16 text_size = oti.font->measure_text(ssize16::max(),spoint16::zero(),oti.text,oti.scale);
    srect16 text_rect = text_size.bounds();
    text_rect.center_inplace((srect16)dsp.bounds());
    draw::text(dsp,text_rect,oti,color_t::purple);
    delay(2000);
    
    //draw::text(dsp,)
}
void loop() {
    int64_t n, a, b, a2, b2, posReal, posImag;
    uint32_t startTime, elapsedTime;

    int32_t
        startReal = (int64_t)((centerReal - rangeReal * 0.5) * (float)(1 << res_bits)),
        startImag = (int64_t)((centerImag + rangeImag * 0.5) * (float)(1 << res_bits)),
        incReal = (int64_t)((rangeReal / (float)pixelWidth) * (float)(1 << res_bits)),
        incImag = (int64_t)((rangeImag / (float)pixelHeight) * (float)(1 << res_bits));

    startTime = millis();
    posImag = startImag;
    auto bt = draw::batch(dsp, dsp.bounds());
    for (int y = 0; y < dsp.dimensions().height; y++) {
        posReal = startReal;
        for (int x = 0; x < dsp.dimensions().width; x++) {
            a = posReal;
            b = posImag;
            for (n = iterations; n > 0; n--) {
                a2 = (a * a) >> res_bits;
                b2 = (b * b) >> res_bits;
                if ((a2 + b2) >= (4 << res_bits))
                    break;
                b = posImag + ((a * b) >> (res_bits - 1));
                a = posReal + a2 - b2;
            }
            display_t::pixel_type px;
            px.native_value = (n * 29) << 8 | (n * 67);
            bt.write(px);

            posReal += incReal;
        }
        posImag -= incImag;
    }
    bt.commit();
    elapsedTime = millis() - startTime;
    Serial.print("Took ");
    Serial.print(elapsedTime);
    Serial.println(" ms");

    rangeReal *= incRange;
    rangeImag *= incRange;

    button_1.update();
    button_2.update();
}