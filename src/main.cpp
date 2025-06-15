/********************************************************************
 *  TriggerinoRTOS.ino  –  top-level sketch (FreeRTOS version)
 ********************************************************************/
#include "Triggerino_helpers.hpp"
#include "hardware/DisplayManager.h"
#include "hardware/ButtonManager.h"
#include "hardware/LEDManager.h"
#include "network/Protocols.h"
#include "core/TriggerinoCore.h"
#include "hardware/SensorManager.h"


/* ---------- RTOS objects ---------------------------------------- */
namespace Triggerino {
  QueueHandle_t netQ   = nullptr;        // general network queue
  QueueHandle_t httpQ  = nullptr;        // separate queue for HTTP
  QueueHandle_t animQ  = nullptr;        // NeoPixel animation queue
  EventGroupHandle_t evtGrp = nullptr;   // event bits

  const uint32_t EVT_OLED_DIRTY  = 0x01;
  const uint32_t EVT_ANIM_UPDATE = 0x02;
}


/* ---------- task prototypes ------------------------------------- */
static void netTask (void *);
static void httpTask(void *);
static void animTask(void *);

/* ================================================================ */
void setup()
{
  Serial.begin(115200);
  Serial.println("\n=== Triggerino RTOS V0.0.1 ===");

  // Create queues & event group first
  Triggerino::netQ   = xQueueCreate(16, sizeof(NetJob*));
  Triggerino::httpQ  = xQueueCreate(16, sizeof(NetJob*));
  Triggerino::animQ  = xQueueCreate(16, sizeof(AnimationCommand));
  Triggerino::evtGrp = xEventGroupCreate();

  // Then continue normal setup
  initFilesystem();
  initNetwork();  // Now queueAnimation() is safe
  loadConfigurationFromFS();
  initButtons();
  Triggerino::setupSensors();
  Triggerino::setupOutputs();
  Triggerino::setupMappings();
  initI2CBus();
  initDisplay();
  initStatusLEDs();
  initWebServer();

  // Start FreeRTOS tasks *after* everything is initialized
  xTaskCreatePinnedToCore(netTask ,  "netTask" ,  8192, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(httpTask,  "httpTask", 8192, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(dispTask,  "dispTask", 2048, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(animTask,  "animTask", 4096, nullptr, 2, nullptr, 0);

  // First animation
  xEventGroupSetBits(Triggerino::evtGrp, Triggerino::EVT_OLED_DIRTY);
  queueAnimation(AnimationType::BOOT, 0, 0, 0, 0, 3000);
}

void loop()   /* core-0 */
{
  webServerLoop();
  Triggerino::readButtons();           // may enqueue jobs / mark display dirty
  Triggerino::readSensors();
  Triggerino::networkHousekeeping();
  delay(1);
}

/* ====================  TASKS  ================================== */
static void netTask(void *)
{
  using namespace Triggerino;
  NetJob *job;

  for (;;)
  {
    if (xQueueReceive(netQ, &job, portMAX_DELAY) == pdTRUE && job)
    {
      switch (job->type)
      {
        case JobType::ART:
          sendArtNet(*job->out, job->payload);
          break;
        case JobType::OSC:
          sendOSC(*job->out, job->payload);
          break;
        case JobType::TCP:
          sendTCP(*job->out, job->payload);
          break;
        case JobType::HTTP:
        case JobType::HTTP_SLOW:
          xQueueSend(httpQ, &job, 0);
          job = nullptr;
          break;
      }

      if (job)
      {
        delete job->payloadDoc;
        delete job;
      }
    }
  }
}

static void httpTask(void *)
{
  using namespace Triggerino;
  NetJob *job;

  for (;;)
  {
    if (xQueueReceive(httpQ, &job, portMAX_DELAY) == pdTRUE && job)
    {
      sendHTTP(*job->out, job->payload);
      delete job->payloadDoc;
      delete job;
    }
  }
}

static void animTask(void *)
{
  using namespace Triggerino;
  AnimationCommand cmd;
  AnimationState animState = {
    AnimationType::IDLE, 0, 0, 0, 0, 0, 0, 0, 0, false
  };
  const TickType_t FRAME_TIME = pdMS_TO_TICKS(16);  // ~60 FPS

  for (;;)
  {
    if (xQueueReceive(animQ, &cmd, 0) == pdTRUE)
    {
      animState.type = cmd.type;
      animState.r = cmd.r;
      animState.g = cmd.g;
      animState.b = cmd.b;
      animState.w = cmd.w;
      animState.duration = cmd.duration;
      animState.startTime = millis();
      animState.frame = 0;
      animState.active = true;
    }

    if (animState.active)
    {
      updateAnimation(animState);
    }
    else
    {
      updateStatusAnimation();
    }

    vTaskDelay(FRAME_TIME);
  }
}