#include "usart.h"
extern uint8_t OLED_DisplayBuf[8][132];
uint8_t rxbuffer[256];
uint8_t rx_count = 0;
uint8_t tx_count = 0;
__IO uint8_t receive_flag = 0;
void dma_config(void)
{
    dma_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_DMA0);

    /* deinitialize DMA channel2 (USART2 rx) */
    dma_deinit(DMA0, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 256;
    dma_init_struct.periph_addr = USART2_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH2);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH2);
}
void usart_config(void)
{
    /* enable USART clock */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_USART2);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 921600U);

#if USARTx_RX_BY_IDLE_DMA
    dma_config(void);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);

    usart_dma_receive_config(USART2, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);
    usart_enable(USART2);
    usart_interrupt_enable(USART2, USART_INT_IDLE);
#else
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);

    // 使能usart0的中断
    usart_interrupt_enable(USART2, USART_INT_RBNE);

    nvic_irq_enable(USART2_IRQn, 2U, 2U);

#endif
}

/// @brief 串口2中断函数
/// @param
void USART2_IRQHandler(void)
{
    static uint8_t p0 = 0, p1 = 0;
#if USARTx_RX_BY_IDLE_DMA
    if (RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE))
    {
        /* clear IDLE flag */
        usart_data_receive(USART2);
        /* number of data received */
        rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH2));
        receive_flag = 1;
        if (tx_count >= 132)
        {
            tx_count = 0;
            p0++;
            if (p0 >= 8)
            {
                p0 = 0;
            }
        }
        OLED_DisplayBuf[p0][p1] = rxbuffer[tx_count];

        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH2);
        dma_transfer_number_config(DMA0, DMA_CH2, 256);
        dma_channel_enable(DMA0, DMA_CH2);
    }

#else
    if (RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE))
    {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);
        uint8_t RxData = usart_data_receive(USART2);
        OLED_DisplayBuf[p0][p1] = RxData;

        /*位置自增*/
        p1++;
        if (p1 >= 132)
        {
            p1 = 0;
            p0++;
            if (p0 >= 8)
            {
                p0 = 0;
            }
        }
    }
#endif
}

#ifdef __GNUC__
int _write(int fd, char *pBuffer, int size)
{
    for (int i = 0; i < size; i++)
    {
        usart_data_transmit(USART2, (uint8_t)pBuffer[i]);
        while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
            ;
    }
    return size;
}

#else
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART2, (uint8_t)ch);
    while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
        ;
    return ch;
}

#endif
/// @brief 自定义串口2 打印函数
/// @param fmt
/// @param
void printf2(const char *fmt, ...)
{
    /* 大小根据需要调整 */
    char buf[256] = {0};
    /* 指向当前参数的一个字符指针 */
    va_list arg;
    /* arg指向可变参数里边的第一个参数 */
    va_start(arg, fmt);
    /* 按照fmt的格式将arg里的值依次转换成字符保存到buf中 */
    vsnprintf(buf, sizeof(buf), fmt, arg);
    va_end(arg);

    for (uint32_t i = 0; i < strlen(buf); i++)
    {
        usart_data_transmit(USART0, buf[i]);
        while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
            ;
    }
    while (usart_flag_get(USART2, USART_FLAG_TC) == RESET)
        ;
}
