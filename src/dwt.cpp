
#include "dwt.hpp"

struct DwtWatchPointRegs {
  uint32_t comp;
  uint32_t mask;
  uint32_t function;
  uint32_t reserved;
};

struct DwtRegs {
  uint32_t cont;
  uint32_t cyccnt;
  uint32_t cpicnt;
  uint32_t exccnt;
  uint32_t sleepcnt;
  uint32_t lsucnt;
  uint32_t foldcnt;
  uint32_t pcsr;
  DwtWatchPointRegs watchpoints[Dwt::MAX_COMPARATORS];
};

volatile DwtRegs s_dwt_regs __attribute__((section(".dwt_regs")));
Dwt* g_dwt = nullptr;

extern "C" void DebugMonHandlerHL(void* sp) { g_dwt->DebugMonitorIsr(sp); }

extern "C" void DebugMon_Handler() {
  asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq r0, msp \n"
      "mrsne r0, psp \n"
      "b DebugMonHandlerHL \n");
}

Dwt::Dwt() { g_dwt = this; }

Dwt::~Dwt() { g_dwt = nullptr; }

void Dwt::DebugMonitorIsr(void* sp) {
  for (uint32_t i = 0; i < MAX_COMPARATORS; i++) {
    if ((s_dwt_regs.watchpoints[i].function & 0x01000000) &&
        (m_callbacks[i] != nullptr)) {
      m_callbacks[i]->WatchpointTriggered(sp);
    }
  }
}

void Dwt::SetWatchPoint(uint32_t id, Params params, Callback* cb) {
  s_dwt_regs.watchpoints[id].comp = reinterpret_cast<uint32_t>(params.address);
  s_dwt_regs.watchpoints[id].mask = params.number_of_bits;
  s_dwt_regs.watchpoints[id].function =
      static_cast<uint32_t>(params.access_type);

  m_callbacks[id] = cb;
}

bool Dwt::Init() {
  auto dhcsr = reinterpret_cast<volatile uint32_t*>(0xE000EDF0);
  if (*dhcsr & 0x1) {
    return false;
  }

  auto demcr = reinterpret_cast<volatile uint32_t*>(0xE000EDFC);
  constexpr uint32_t mon_en_bit = 16;
  constexpr uint32_t trace_ena_bit = 24;
  *demcr = (1 << mon_en_bit) | (1 << trace_ena_bit);

  // Priority for DebugMonitor Exception is bits[7:0].
  // We will use the lowest priority so other ISRs can
  // fire while in the DebugMonitor Interrupt
  auto shpr3 = reinterpret_cast<volatile uint32_t*>(0xE000ED20);
  *shpr3 = 0xff;

  return true;
}
