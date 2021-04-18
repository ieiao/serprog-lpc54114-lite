#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"

#define BOARD_NAME "LPC54114-LITE"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void board_uart_init();

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
