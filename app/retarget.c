#include <stdint.h>
#include "fsl_usart.h"

int _write (int fd, const void *buf, size_t count)
{
    uint8_t *p = (uint8_t *)buf;
    (void) fd;
    USART_WriteBlocking(USART0, p, count);
    if (*(p+count-1) == '\n')
        USART_WriteByte(USART0, '\r');
    return count;
}
