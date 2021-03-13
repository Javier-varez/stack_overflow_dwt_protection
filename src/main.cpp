
#include <cstdint>
#include <cstdio>

#include "cortex_m_hal/systick.h"
#include "dwt.hpp"
#include "postform/rtt_logger.h"

Postform::RttLogger logger;
SysTick& systick = SysTick::getInstance();

class WatchpointCb : public Dwt::Callback {
  void WatchpointTriggered(void* sp) {
    LOG_ERROR(&logger, "Stack overflow detected at addr! %p", sp);
    while (true) {
    }
  }
};

void TriggerStackOverflow() {
  uint8_t buffer[64];
  memset(buffer, 0, sizeof(buffer));

  LOG_DEBUG(&logger, "TriggerStackOverflow %p", buffer);
  TriggerStackOverflow();
}

int main() {
  Dwt dwt;
  WatchpointCb watchpoint_cb;

  constexpr uint32_t systick_clk_hz = 64'000'000;
  systick.init(systick_clk_hz);
  systick.delay(SysTick::TICKS_PER_SECOND);

  if (!dwt.Init()) {
    LOG_ERROR(&logger, "Error initializing DWT");
    while (true) {
    }
  }

  extern uint32_t __end__;
  uint32_t* safety_zone = &__end__ + 128;
  dwt.SetWatchPoint(1, Dwt::Params{safety_zone, Dwt::AccessType::WO, 0},
                    &watchpoint_cb);

  LOG_DEBUG(&logger, "Triggering stack overflow");
  TriggerStackOverflow();

  return 0;
}
