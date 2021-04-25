#include "fsl_device_registers.h"
#include "fsl_power.h"
#include "fsl_reset.h"
#include "fsl_spi.h"
#include "LPC54114_cm4.h"

#include "board.h"
#include "pin_mux.h"
#include "tusb.h"
#include "serprog.h"

#include "FreeRTOS.h"
#include "task.h"

#define PGMNAME "lpc-serprog"
#define CMD_MAP (\
    (1 << S_CMD_NOP)        | \
    (1 << S_CMD_Q_IFACE)    | \
    (1 << S_CMD_Q_CMDMAP)   | \
    (1 << S_CMD_Q_PGMNAME)  | \
    (1 << S_CMD_Q_SERBUF)   | \
    (1 << S_CMD_Q_BUSTYPE)  | \
    (1 << S_CMD_SYNCNOP)    | \
    (1 << S_CMD_S_BUSTYPE)  | \
    (1 << S_CMD_O_SPIOP)      \
)
#define BUS_SPI (1 << 3)

#define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)

StackType_t  usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;

#define CDC_STACK_SZIE      USBD_STACK_SIZE
StackType_t  cdc_stack[CDC_STACK_SZIE];
StaticTask_t cdc_taskdef;

uint8_t sbuf[64];
uint8_t rbuf[1024];

void usb_device_task(void* param);
void cdc_task(void *param);

void USB0_IRQHandler(void)
{
    tud_int_handler(0);
}

int main(void)
{
    spi_master_config_t masterConfig = {0};

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    CLOCK_SetupFROClocking(48000000U); /*!< Set up high frequency FRO output to selected frequency */

    CLOCK_AttachClk(kFRO_HF_to_FLEXCOMM2);
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
    SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.direction    = kSPI_MsbFirst;
    masterConfig.polarity     = kSPI_ClockPolarityActiveHigh;
    masterConfig.phase        = kSPI_ClockPhaseFirstEdge;
    masterConfig.baudRate_Bps = 24000000;
    masterConfig.sselNum      = (spi_ssel_t)3;
    masterConfig.sselPol      = (spi_spol_t)kSPI_SpolActiveAllLow;
    SPI_MasterInit(SPI2, &masterConfig, CLOCK_GetFlexCommClkFreq(2));

    NVIC_SetPriority(USB0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*Turn on USB Phy */
    CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf)); /* enable USB IP clock */

    (void) xTaskCreateStatic( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);
    (void) xTaskCreateStatic( cdc_task, "cdc", CDC_STACK_SZIE, NULL, configMAX_PRIORITIES-2, cdc_stack, &cdc_taskdef);

    vTaskStartScheduler();

    while (1);
}

void usb_device_task(void* param)
{
    (void) param;

    tusb_init();

    while (1) {
        tud_task();
    }
}

void handle_command(uint8_t command)
{
    uint8_t tmp8;
    uint32_t l = 0;
    uint32_t i = 0;
    uint32_t slen, rlen;
    uint32_t offset = 0;
    spi_transfer_t xfer = {0};

    switch(command) {
    case S_CMD_NOP:
        rbuf[l++] = S_ACK;
        break;
    
    case S_CMD_Q_IFACE:
        rbuf[l++] = S_ACK;
        rbuf[l++] = 0x1;
        rbuf[l++] = 0x0;
        break;

    case S_CMD_Q_CMDMAP:
        rbuf[l++] = S_ACK;
        rbuf[l++] = CMD_MAP & 0xff;
        rbuf[l++] = (CMD_MAP >> 8) & 0xff;
        rbuf[l++] = (CMD_MAP >> 16) & 0xff;
        rbuf[l++] = (CMD_MAP >> 24) & 0xff;
        while (l < (32+1))
            rbuf[l++] = 0;
        break;
    
    case S_CMD_Q_PGMNAME:
        rbuf[l++] = S_ACK;
        i = 0;
        while (PGMNAME[i])
            rbuf[l++] = PGMNAME[i++];
        for (; i < 16; i++)
            rbuf[l++] = 0;
        break;

    case S_CMD_Q_SERBUF:
        rbuf[l++] = S_ACK;
        rbuf[l++] = 0xff;
        rbuf[l++] = 0xff;
        break;
    
    case S_CMD_Q_BUSTYPE:
        rbuf[l++] = S_ACK;
        rbuf[l++] = BUS_SPI;
        break;

    case S_CMD_SYNCNOP:
        rbuf[l++] = S_NAK;
        rbuf[l++] = S_ACK;
        break;

    case S_CMD_S_BUSTYPE:
        while(tud_cdc_available() == 0);
        tud_cdc_read(&tmp8, 1);
        if ( (tmp8|BUS_SPI) == BUS_SPI)
            rbuf[l++] = S_ACK;
        else
            rbuf[l++] = S_NAK;
        break;

    case S_CMD_O_SPIOP:
        while ( tud_cdc_available() < 6 );
        tud_cdc_read(sbuf, 6);
        slen = sbuf[0] | (sbuf[1] << 8) | (sbuf[2] << 16);
        rlen = sbuf[3] | (sbuf[4] << 8) | (sbuf[5] << 16);

        tmp8 = S_ACK;
        tud_cdc_write(&tmp8, 1);
        tud_cdc_write_flush();

        xfer.configFlags = 0;
        while (slen > 0) {
            l = slen > sizeof(sbuf) ? sizeof(sbuf) : slen;
            while (tud_cdc_available() == 0);
            l = tud_cdc_read(sbuf, l);
            if (l > 0) {
                xfer.txData = sbuf;
                xfer.rxData = rbuf;
                xfer.dataSize = l;
                if ((slen - l) == 0 && rlen == 0)
                    xfer.configFlags |= kSPI_FrameAssert;
                SPI_MasterTransferBlocking(SPI2, &xfer);
                slen -= l;
            }
        }

        SPI2->FIFOCFG |= (1 << 17);
        xfer.configFlags = 0;
        while (rlen > 0) {
            offset = 0;
            l = rlen > sizeof(rbuf) ? sizeof(rbuf) : rlen;
            xfer.txData = sbuf;
            xfer.rxData = rbuf;
            xfer.dataSize = l;
            if ((rlen - l) == 0)
                xfer.configFlags |= kSPI_FrameAssert;
            SPI_MasterTransferBlocking(SPI2, &xfer);
            rlen -= l;
            while ( l > 0 ) {
                i = tud_cdc_write_available();
                if (i == 0)
                    continue;
                i = l > i ? i : l;
                tud_cdc_write(rbuf + offset, i);
                tud_cdc_write_flush();
                offset += i;
                l -= i;
            }
        }

        l = 0;

        break;

    default:
        rbuf[l++] = S_NAK;
        break;
    }

    if (l > 0) {
        while(tud_cdc_write_available() < l)
            vTaskDelay(pdMS_TO_TICKS(10));
        tud_cdc_write(rbuf, l);
        tud_cdc_write_flush();
    }
}

void cdc_task(void *param)
{
    (void) param;
    while (1) {
        if (tud_cdc_available()) {
            uint8_t command;
            tud_cdc_read(&command, 1);
            handle_command(command);
        }
    }
}
