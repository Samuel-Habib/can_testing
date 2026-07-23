#include "main.h"
#include "stm32h7xx_it.h"
#include <assert.h>

/* PDU testing
 * Normal operation --> results in "PDU Normal" message in the buffer
 * Transient Investigation --> "Current greather than normal"
 * Transient Result: --> "DISCONNECT: Sustained current exceeded threshold"
 *                   --> "ALLOWED: Sustained current did not exceed threshold"
 * Circular Buffer testing
 *
 * */

// an irq is just a function so you can call it if you want to trigger it
// manually, you don't need to assert a flag or anything like that
//

/* typedef enum { */
/**/
/* } adc_state; */
/**/
int adc_testing() {

  assert(ADC1->DR = SHORT_CIRCUIT_THRESHOLD);
  assert(ADC1->DR = CURRENT_RATING);

  ADC_IRQHandler();
  DMA1_Stream1_IRQHandler();
  return 0;
}

typedef enum { adc_state1 } adc_state;
