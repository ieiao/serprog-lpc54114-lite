#include "fsl_common.h"
#include "fsl_iocon.h"
#include "pin_mux.h"

void BOARD_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iocon);

    /* USB0_VBUS */
    const uint32_t port1_pin6_config = (7 << 0) | (1 << 7) | (1 << 8);
    IOCON_PinMuxSet(IOCON, 1U, 6U, port1_pin6_config);

    /* fc2 ssel3 */
    const uint32_t port0_pin2_config = (2 << 0) | (1 << 7) | (1 << 8);
    IOCON_PinMuxSet(IOCON, 0U, 2U, port0_pin2_config);
    /* fc2 mosi */
    const uint32_t port0_pin8_config = (1 << 0) | (1 << 7) | (1 << 8);
    IOCON_PinMuxSet(IOCON, 0U, 8U, port0_pin8_config);
    /* fc2 miso */
    const uint32_t port0_pin9_config = (1 << 0) | (1 << 7) | (1 << 8);
    IOCON_PinMuxSet(IOCON, 0U, 9U, port0_pin9_config);
    /* fc2 clk */
    const uint32_t port0_pin10_config = (1 << 0) | (1 << 7) | (1 << 8);
    IOCON_PinMuxSet(IOCON, 0U, 10U, port0_pin10_config);
}
