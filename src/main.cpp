
#include <cstdint>
#include <cstdio>

#include "cortex_m_hal/systick.h"
#include "postform/rtt_logger.h"

Postform::RttLogger logger;
SysTick& systick = SysTick::getInstance();

void TriggerStackOverflow() {
  uint8_t buffer[128];
  memset(buffer, 0, sizeof(buffer));

  LOG_DEBUG(&logger, "Stack pointer %p", buffer);
  TriggerStackOverflow();
}

void ConfigureDebugMonitor() {
  auto dhcsr = reinterpret_cast<volatile uint32_t*>(0xE000EDF0);
  if (*dhcsr & 0x1) {
    LOG_ERROR(&logger, "Halting Debug Enabled - "
                       "Can't Enable Monitor Mode Debug!");
    return;
  }

  auto demcr = reinterpret_cast<volatile uint32_t*>(0xE000EDFC);
  constexpr uint32_t mon_en_bit = 16;
  *demcr |= 1 << mon_en_bit;

  // Priority for DebugMonitor Exception is bits[7:0].
  // We will use the lowest priority so other ISRs can
  // fire while in the DebugMonitor Interrupt
  auto shpr3 = reinterpret_cast<volatile uint32_t*>(0xE000ED20);
  *shpr3 = 0xff;

  LOG_DEBUG(&logger, "Monitor Mode Debug Enabled!");
  return;
}

int main() {
  constexpr uint32_t systick_clk_hz = 8'000'000;
  systick.init(systick_clk_hz);
  ConfigureDebugMonitor();

  LOG_DEBUG(&logger, "Triggering stack overflow");
  TriggerStackOverflow();

  return 0;
}
