#include <stdint.h>
#include "fsl_common.h"
#include "fsl_usart.h"
#include "clock_config.h"
#include "board.h"

void board_uart_init()
{
    usart_config_t config;
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx = true;
    USART_Init(USART0, &config, CLOCK_GetFlexCommClkFreq(0));
}
