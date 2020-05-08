#include "esp_pm.h"
#include <soc/rtc.h>
#include "canvas.h"
#include "pal_grayscale.h"
#include "constants.h"

CompositeOutput * composite;
Canvas * canvas;

void setup()
{
  Serial.begin(115200);

  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);

  // TODO se débarasser de cette demi-résolution.
  composite = new CompositeOutput(WIDTH * 2, HEIGHT * 2);
  canvas = new Canvas();

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeTask, "compositeCoreTask", 1024, NULL, 1, NULL, 0);
  //rendering the actual graphics in the main loop is done on the second core by default

}

void compositeTask(void *data)
{
  while (true)
  {
    composite->sendFrameHalfResolution(canvas->get_frame());
    // TODO - Send VSync signal
  }
}

void loop()
{
  //clearing background and starting to draw
  canvas->begin(0);
  canvas->line(0, 0, 10, 10, 127);
  for(int i = 0; i < WIDTH; i = i + 2) {
    canvas->line(i, 0, i, HEIGHT - 1, 31);
  }

  for(int i = 0; i < HEIGHT; i = i + 2) {
    canvas->line(0, i, WIDTH - 1, i, 31);
  }

  for(int i = 0; i < WIDTH; i = i + 10) {
    canvas->line(i, 0, i, HEIGHT - 1, 127);
  }

  for(int i = 0; i < HEIGHT; i = i + 10) {
    canvas->line(0, i, WIDTH - 1, i, 127);
  }
  canvas->line(WIDTH - 1, 0, WIDTH - 1, HEIGHT - 1, 127);
  canvas->line(WIDTH - 1, HEIGHT - 1, 0, HEIGHT - 1, 127);

  // TODO - Wait for the VSync signal
  canvas->end();
}
