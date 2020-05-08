#include "esp_pm.h"
#include <soc/rtc.h>
#include "canvas.h"
#include "pal_grayscale.h"

CompositeOutput * composite;
int graphicsXres, graphicsYres;
Canvas * canvas;

void setup()
{
  Serial.begin(115200);

  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);

  // TODO se débarasser de cette demi-résolution.
  composite = new CompositeOutput(320, 240);
  graphicsXres = composite->width / 2;
  graphicsYres = composite->height / 2;
  canvas = new Canvas(graphicsXres, graphicsYres);

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

  for(int i = 0; i < graphicsXres; i = i + 2) {
    canvas->line(i, 0, i, graphicsYres, 31);
  }

  for(int i = 0; i < graphicsYres; i = i + 2) {
    canvas->line(0, i, graphicsXres, i, 31);
  }

  for(int i = 0; i < graphicsXres; i = i + 10) {
    canvas->line(i, 0, i, graphicsYres, 127);
  }

  for(int i = 0; i < graphicsYres; i = i + 10) {
    canvas->line(0, i, graphicsXres, i, 127);
  }

  canvas->line(graphicsXres - 1, 0, graphicsXres - 1, graphicsYres - 1, 127);
  canvas->line(graphicsXres - 1, graphicsYres - 1, 0, graphicsYres - 1, 127);

  // TODO - Wait for the VSync signal
  canvas->end();
}
