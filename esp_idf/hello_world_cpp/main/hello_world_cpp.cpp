#include <stdio.h>
#include <bb_epaper.h>
BBEPAPER bbep(EP81_SPECTRA_1024x576);

#define DC_PIN    14
#define BUSY_PIN  13
#define RESET_PIN 9
#define CS_PIN    10
#define CS_PIN2   8

#define POWER_PIN -1
#define SCK_PIN   12
#define MOSI_PIN  11

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 576

extern const bb_lv_font_t bpg_square140;

extern "C" void app_main(void)
{
	bbep.initIO(DC_PIN, RESET_PIN, BUSY_PIN, CS_PIN, MOSI_PIN, SCK_PIN, 8000000);
	bbep.setCS2(CS_PIN2);
  bbep.allocBuffer();
  bbep.fillScreen(BBEP_WHITE);
  bbep.setTextColor(BBEP_BLACK, BBEP_WHITE);
  bbep.drawStringNew(&bpg_square140, "AVy:AV:..", 100,200, true);
  bbep.drawStringNew(&bpg_square140, "AVy:AV:..", 100,400, false);
  bbep.drawRect(100,200,900,bpg_square140.line_height, BBEP_BLACK);
  bbep.drawLine(100,200+bpg_square140.line_height+bpg_square140.underline_position+bpg_square140.base_line, 900,200+bpg_square140.line_height+bpg_square140.underline_position+bpg_square140.base_line,BBEP_BLACK);
  bbep.setFont(FONT_12x16);
  bbep.drawString("World!", 10, 20);
  bbep.drawRect(10,20,400,20,BBEP_BLACK);

  bbep.writePlane();
  bbep.refresh(REFRESH_FULL);
  bbep.sleep(1);
}
