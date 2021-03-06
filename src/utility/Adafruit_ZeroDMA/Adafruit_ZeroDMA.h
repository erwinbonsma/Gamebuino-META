#ifndef _ADAFRUIT_ZERODMA_H_
#define _ADAFRUIT_ZERODMA_H_

#include "../../config/config.h"
#include "utility/dma.h"

namespace Gamebuino_Meta {

void cpu_irq_enter_critical(void);
void cpu_irq_leave_critical(void);

// Status codes returned by some DMA functions and/or held in
// a channel's jobStatus variable.
enum ZeroDMAstatus {
	DMA_STATUS_OK = 0,
	DMA_STATUS_ERR_NOT_FOUND,
	DMA_STATUS_ERR_NOT_INITIALIZED,
	DMA_STATUS_ERR_INVALID_ARG,
	DMA_STATUS_ERR_IO,
	DMA_STATUS_ERR_TIMEOUT,
	DMA_STATUS_BUSY,
	DMA_STATUS_SUSPEND,
	DMA_STATUS_ABORTED,
	DMA_STATUS_JOBSTATUS = -1 // For printStatus() function
};

class Adafruit_ZeroDMA {
 public:
  Adafruit_ZeroDMA(void);

  // DMA channel functions
  ZeroDMAstatus   allocate(void), // Allocates DMA channel
                  startJob(void),
                  free(void);     // Deallocates DMA channel
  void            trigger(void),
                  setTrigger(uint8_t trigger),
                  setAction(dma_transfer_trigger_action action),
                  setCallback(void (*callback)(Adafruit_ZeroDMA *) = NULL,
                    dma_callback_type type = DMA_CALLBACK_TRANSFER_DONE),
                  loop(bool flag),
                  suspend(void),
                  resume(void),
                  abort(void),
                  printStatus(ZeroDMAstatus s = DMA_STATUS_JOBSTATUS);

  // DMA descriptor functions
  DmacDescriptor *addDescriptor(void *src, void *dst, uint32_t count = 0,
                    dma_beat_size size = DMA_BEAT_SIZE_BYTE,
                    bool srcInc = true, bool dstInc = true, 
					uint32_t stepSize = DMA_ADDRESS_INCREMENT_STEP_SIZE_1, 
					bool stepSel = DMA_STEPSEL_DST);
  void            changeDescriptor(DmacDescriptor *d, void *src = NULL,
                    void *dst = NULL, uint32_t count = 0);

  void            _IRQhandler(uint8_t flags); // DO NOT TOUCH

  uint8_t                     channel;

 protected:
  volatile enum ZeroDMAstatus jobStatus;
  bool                        hasDescriptors;
  bool                        loopFlag;
  uint8_t                     peripheralTrigger;
  dma_transfer_trigger_action triggerAction;
  void                      (*callback[DMA_CALLBACK_N])(Adafruit_ZeroDMA *);
};

} // namespace Gamebuino_Meta

#endif // _ADAFRUIT_ZERODMA_H_
