#include <stdio.h>
#include <bb_epaper.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
extern const bb_lv_font_t HelveticaNeue30;

extern "C" void app_main(void)
{

  const char testtext[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer tempus sem sed rutrum semper. Nam rutrum magna metus, nec egestas nisl suscipit vel. Morbi eget aliquam erat, ac accumsan sem. Vivamus eu ipsum sem. Curabitur at lacinia sem. Praesent ultricies mi sed blandit ullamcorper. Aenean ac leo fringilla, vestibulum ante sed, ultrices libero. Nam convallis ultrices enim. Duis vel placerat urna, vel maximus ipsum.\n\nNam ac porttitor massa. Donec mattis malesuada purus, at feugiat lacus pulvinar egestas. Nullam nisl tellus, lacinia a mi eu, auctor efficitur erat. Etiam dui ex, commodo et sem vitae, molestie sollicitudin elit. Ut metus massa, accumsan vitae elementum et, imperdiet sed lectus. Donec vestibulum lacus vel eros pretium, et laoreet arcu aliquam. Integer pellentesque erat viverra magna eleifend, a aliquam ante placerat. Vivamus vehicula, lectus sit amet commodo condimentum, eros ipsum semper orci, at ultrices justo orci non lectus. Sed gravida, elit ac vulputate sodales, arcu augue commodo est, nec scelerisque augue felis eu tellus. Duis nisi nulla, pulvinar vitae pretium non, aliquet at purus. Nam vel arcu ipsum. Sed placerat vel nisl a porta. Proin a posuere arcu.\n\nFusce et nulla at sapien interdum vehicula vel sagittis odio. Donec suscipit purus turpis, sit amet condimentum quam finibus id. Integer sed erat imperdiet, iaculis odio in, vulputate erat. Nulla et justo sed metus hendrerit maximus eget vitae leo. Nam at ex sollicitudin, euismod justo ut, vehicula urna. Vestibulum vel eros non enim lobortis vehicula ut et sem. Integer consectetur aliquam lorem, sed aliquam turpis porttitor quis. Sed nec dolor dapibus, euismod leo non, eleifend nulla.";
	bbep.initIO(DC_PIN, RESET_PIN, BUSY_PIN, CS_PIN, MOSI_PIN, SCK_PIN, 8000000);
	bbep.setCS2(CS_PIN2);
  bbep.allocBuffer();
  bbep.fillScreen(BBEP_WHITE);
  bbep.setTextColor(BBEP_BLACK, BBEP_WHITE);
  bbep.drawStringNew(&HelveticaNeue30, testtext, 10,10, true, 1024-20);
  bbep.writePlane();
  bbep.refresh(REFRESH_FULL);

  vTaskDelay(10000 / portTICK_PERIOD_MS);

  bbep.fillScreen(BBEP_WHITE);
  bbep.drawStringNew(&bpg_square140, "AVy:AV:..", 100,200, true);
  bbep.drawStringNew(&bpg_square140, "AVy:AV:..", 100,400, false);
  bbep.drawRect(100,200,900,bpg_square140.line_height, BBEP_BLACK);
  bbep.drawLine(100,200+bpg_square140.line_height+bpg_square140.underline_position+bpg_square140.base_line, 900,200+bpg_square140.line_height+bpg_square140.underline_position+bpg_square140.base_line,BBEP_BLACK);
  bbep.setFont(FONT_12x16);
  bbep.drawString("World!\n\rTEST123", 10, 20);
  bbep.drawRect(10,20,400,20,BBEP_BLACK);
  bbep.writePlane();
  bbep.refresh(REFRESH_FULL);
  bbep.sleep(1);
}
