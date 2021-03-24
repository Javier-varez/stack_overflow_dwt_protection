
#include "dwt.hpp"

struct DwtWatchPointRegs {
  uint32_t dwt_comp;
  union {
    uint32_t reg;
    struct {
      uint32_t mask : 5;
    };
  } dwt_mask;
  union {
    uint32_t reg;
    struct {
      Dwt::AccessType function : 4;
      uint32_t : 1;
      uint32_t emitrange : 1;
      uint32_t : 1;
      uint32_t cycmatch : 1;
      uint32_t datavmatch : 1;
      uint32_t lnk1ena : 1;
      uint32_t datavsize : 2;
      uint32_t datavaddr0 : 4;
      uint32_t datavaddr1 : 4;
      uint32_t : 4;
      uint32_t matched : 1;
    };
  } dwt_function;
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

struct ScbRegs {
  uint32_t cpuid;
  uint32_t icsr;
  uint32_t vtor;
  uint32_t aircr;
  uint32_t scr;
  uint32_t ccr;
  union {
    uint32_t reg;
    struct {
      uint32_t mem_manage_prio : 8;
      uint32_t bus_fault_prio : 8;
      uint32_t usage_fault_prio : 8;
    };
  } shpr1;
  union {
    uint32_t reg;
    struct {
      uint32_t : 8;
      uint32_t : 8;
      uint32_t : 8;
      uint32_t svc_prio : 8;
    };
  } shpr2;
  union {
    uint32_t reg;
    struct {
      uint32_t debug_monitor_prio : 8;
      uint32_t : 8;
      uint32_t pendsv_prio : 8;
      uint32_t systick_prio : 8;
    };
  } shpr3;
  uint32_t shcsr;
  uint32_t cfsr;
  uint32_t hfsr;
  uint32_t dfsr;
  uint32_t mmfar;
  uint32_t bfar;
  uint32_t afsr;
  uint32_t reserved[18];
  uint32_t cpacr;
};

struct ScbDebugRegs {
  struct Dhcsr {
    uint32_t c_debugen : 1;
    uint32_t c_halt : 1;
    uint32_t c_step : 1;
    uint32_t c_maskints : 1;
    uint32_t : 1;
    uint32_t c_snapstall : 1;
    uint32_t : 10;
    uint32_t s_regrdy : 1;
    uint32_t s_halt : 1;
    uint32_t s_sleep : 1;
    uint32_t s_lockup : 1;
    uint32_t : 4;
    uint32_t s_retire_st : 1;
    uint32_t s_reset_st : 1;
  } dhcsr;
  uint32_t dcsrs;
  uint32_t dcrdr;
  union {
    uint32_t reg;
    struct {
      uint32_t vc_corereset : 1;
      uint32_t : 3;
      uint32_t vc_mmerr : 1;
      uint32_t vc_nocperr : 1;
      uint32_t vc_chkerr : 1;
      uint32_t vc_staterr : 1;
      uint32_t vc_buserr : 1;
      uint32_t vc_interr : 1;
      uint32_t vc_harderr : 1;
      uint32_t : 5;
      uint32_t mon_en : 1;
      uint32_t mon_pend : 1;
      uint32_t mon_step : 1;
      uint32_t mon_req : 1;
      uint32_t : 4;
      uint32_t trcena : 1;
    };
  } demcr;
};

volatile DwtRegs s_dwt_regs __attribute__((section(".dwt_regs")));
volatile ScbRegs s_scb_regs __attribute__((section(".scb_regs")));
volatile ScbDebugRegs s_scb_debug_regs
    __attribute__((section(".scb_debug_regs")));

static Dwt* s_dwt = nullptr;

extern "C" void DebugMonHandlerHL(void* sp) { s_dwt->DebugMonitorIsr(sp); }

extern "C" void DebugMon_Handler() {
  asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq r0, msp \n"
      "mrsne r0, psp \n"
      "b DebugMonHandlerHL \n");
}

Dwt::Dwt() { s_dwt = this; }

Dwt::~Dwt() { s_dwt = nullptr; }

void Dwt::DebugMonitorIsr(void* sp) {
  for (uint32_t i = 0; i < MAX_COMPARATORS; i++) {
    if ((s_dwt_regs.watchpoints[i].dwt_function.matched) &&
        (m_callbacks[i] != nullptr)) {
      m_callbacks[i]->WatchpointTriggered(i, sp);
    }
  }
}

void Dwt::SetWatchPoint(uint32_t id, Params params, Callback* cb) {
  s_dwt_regs.watchpoints[id].dwt_comp =
      reinterpret_cast<uint32_t>(params.address);
  s_dwt_regs.watchpoints[id].dwt_mask.mask = params.number_of_bits;
  s_dwt_regs.watchpoints[id].dwt_function.function = params.access_type;

  m_callbacks[id] = cb;
}

bool Dwt::Init() {
  if (s_scb_debug_regs.dhcsr.c_debugen) {
    return false;
  }

  s_scb_debug_regs.demcr.mon_en = 1;
  s_scb_debug_regs.demcr.trcena = 1;

  s_scb_regs.shpr3.debug_monitor_prio = 0xff;

  return true;
}
