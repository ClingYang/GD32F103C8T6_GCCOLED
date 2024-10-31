#include "gd32f10x.h"
#include "systick.h"
#include "OLED.h"
#include "usart.h"
int main()
{

    systick_config();
    usart_config();
    printf("\n\rPlease send data less than 256 bytes:\n\r");
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(40, 24, "KUN UI", OLED_8X16);
    OLED_Update();
    while (1)
    {
        OLED_Update(); // 不断将OLED显存数组的内容刷新到OLED屏幕
    }
}
