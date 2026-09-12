#include "bsp_lcd.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
static SemaphoreHandle_t write_gram_sem;
//SPI3	
#define NSS_PORT GPIOA
#define NSS_PIN GPIO_Pin_15

#define DC_PORT GPIOD
#define DC_PIN GPIO_Pin_13

#define BL_PORT GPIOD
#define BL_PIN GPIO_Pin_12

#define SCK_PORT GPIOB
#define SCK_PIN GPIO_Pin_3

#define MOSI_PORT GPIOB
#define MOSI_PIN GPIO_Pin_5
static void st7789_gpio_init(void)
{
		GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    
		// 初始化引脚状态：BL默认低，其他默认高
    GPIO_SetBits(NSS_PORT, NSS_PIN);
		GPIO_SetBits(DC_PORT, DC_PIN);
		GPIO_SetBits(BL_PORT, BL_PIN);
    GPIO_ResetBits(BL_PORT, BL_PIN);
    
		// 初始化IO口
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Pin = NSS_PIN;
    GPIO_Init(NSS_PORT, &GPIO_InitStruct);
	
		GPIO_InitStruct.GPIO_Pin = DC_PIN;
    GPIO_Init(DC_PORT, &GPIO_InitStruct);
	
		GPIO_InitStruct.GPIO_Pin = BL_PIN;
    GPIO_Init(BL_PORT, &GPIO_InitStruct);
		
    // 初始化SPI的IO
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Pin = SCK_PIN;
    GPIO_Init(SCK_PORT, &GPIO_InitStruct);
		
    GPIO_InitStruct.GPIO_Pin = MOSI_PIN;
    GPIO_Init(MOSI_PORT, &GPIO_InitStruct);
}

// 时钟42MHz，预分频2得到21MHz的SPI时钟
static void st7789_spi_init(void)
{
		SPI_InitTypeDef SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI3, &SPI_InitStruct);
    SPI_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(SPI3, ENABLE);
}

// DMA初始化：SPI3对应DMA1通道0，流5或流7，这里选择流5
static void st7789_dma_init(void)
{
		DMA_InitTypeDef DMA_InitStruct;
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_Channel = DMA_Channel_0;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&SPI3->DR;
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_INC8;
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
    DMA_Init(DMA1_Stream5, &DMA_InitStruct);
}

// DMA中断初始化
static void st7789_int_init(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_SetPriority(DMA1_Stream5_IRQn, 5);
}
static void st7789_set_backlight(bool on)
{
    GPIO_WriteBit(BL_PORT, BL_PIN, on ? Bit_SET : Bit_RESET);
}
static void st7789_init_display(void);
// lcd初始化
void st7789_init(void)
{
	write_gram_sem = xSemaphoreCreateBinary();
	st7789_spi_init();
	st7789_dma_init();
	st7789_int_init();
	st7789_gpio_init();
	
	st7789_init_display();
}

// 向对应寄存器写入数据
static void st7789_write_register(uint8_t reg, uint8_t data[], uint16_t length)
{
    SPI_DataSizeConfig(SPI3, SPI_DataSize_8b);
    
    GPIO_ResetBits(NSS_PORT, NSS_PIN);
    
    GPIO_ResetBits(DC_PORT, DC_PIN);
    SPI_SendData(SPI3, reg);
    while (SPI_GetFlagStatus(SPI3, SPI_FLAG_TXE) == RESET);
    while (SPI_GetFlagStatus(SPI3, SPI_FLAG_BSY) != RESET);
    
    GPIO_SetBits(DC_PORT, DC_PIN);
    for (uint16_t i = 0; i < length; i++)
    {
        SPI_SendData(SPI3, data[i]);
        while (!SPI_GetFlagStatus(SPI3, SPI_FLAG_TXE));
    }
    while (SPI_GetFlagStatus(SPI3, SPI_FLAG_BSY) != RESET);
    
    GPIO_SetBits(NSS_PORT, NSS_PIN);
}

static void st7789_write_gram(uint8_t data[], uint32_t length, bool singlecolor)
{
    SPI_DataSizeConfig(SPI3, SPI_DataSize_16b);
    
    GPIO_ResetBits(NSS_PORT, NSS_PIN);
    GPIO_SetBits(DC_PORT, DC_PIN);
    
    length >>= 1;
    
    do
    {
        uint32_t chunk_size = length < 65535 ? length : 65535;

        if (singlecolor) DMA1_Stream5->CR &= ~DMA_SxCR_MINC;
        else             DMA1_Stream5->CR |= DMA_SxCR_MINC;
        DMA1_Stream5->M0AR = (uint32_t)data;
        DMA1_Stream5->NDTR = chunk_size;

        DMA_Cmd(DMA1_Stream5, ENABLE);
        xSemaphoreTake(write_gram_sem, portMAX_DELAY);
        
        if (!singlecolor)
            data += chunk_size * 2;
        length -= chunk_size;
    } while (length > 0);
    
    while (SPI_GetFlagStatus(SPI3, SPI_FLAG_BSY) != RESET);

    GPIO_SetBits(NSS_PORT, NSS_PIN);
}

static void st7789_init_display(void)
{
    vTaskDelay(pdMS_TO_TICKS(5));
    
    st7789_write_register(0x36, (uint8_t[]){0x00}, 1);
    st7789_write_register(0x3A, (uint8_t[]){0x05}, 1);
    st7789_write_register(0xB2, (uint8_t[]){0x0C,0x0C,0x00,0x33,0x33}, 1);

    st7789_write_register(0xB7, (uint8_t[]){0x35}, 1);

    st7789_write_register(0xBB, (uint8_t[]){0x19}, 1);
    st7789_write_register(0xC0, (uint8_t[]){0x2C}, 1);
    st7789_write_register(0xC2, (uint8_t[]){0x01}, 1);
    st7789_write_register(0xC3, (uint8_t[]){0x12}, 1);
    st7789_write_register(0xC4, (uint8_t[]){0x20}, 1);
    st7789_write_register(0xC6, (uint8_t[]){0x0F}, 1);
    st7789_write_register(0xD0, (uint8_t[]){0xA4,0xA1}, 2);
    st7789_write_register(0xD6, (uint8_t[]){0xA1}, 1);
    st7789_write_register(0xE0, (uint8_t[]){0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23}, 14);
    st7789_write_register(0xE1, (uint8_t[]){0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23}, 14);
    st7789_write_register(0x21, NULL, 0);
    st7789_write_register(0x11, NULL, 0);// 退出睡眠模式
    vTaskDelay(pdMS_TO_TICKS(120));
    st7789_write_register(0x29, NULL, 0);
		
		st7789_fill_color(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, 0x0000);
		st7789_set_backlight(true);
}
static bool in_screen_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (x1 >= ST7789_WIDTH || y1 >= ST7789_HEIGHT)
        return false;
    if (x2 >= ST7789_WIDTH || y2 >= ST7789_HEIGHT)
        return false;
    if (x1 > x2 || y1 > y2)
        return false;

    return true;
}
static void st7789_set_range_and_prepare_gram(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_write_register(0x2A, (uint8_t[]){(x1 >> 8) & 0xff, x1 & 0xff, (x2 >> 8) & 0xff, x2 & 0xff}, 4);
    st7789_write_register(0x2B, (uint8_t[]){(y1 >> 8) & 0xff, y1 & 0xff, (y2 >> 8) & 0xff, y2 & 0xff}, 4);
    st7789_write_register(0x2C, NULL, 0);
}
void st7789_fill_color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (!in_screen_range(x1, y1, x2, y2))
        return;
    
    st7789_set_range_and_prepare_gram(x1, y1, x2, y2);
    
    uint32_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    st7789_write_gram((uint8_t *)&color, pixels * 2, true);
}

// 绘制字符
static void st7789_draw_font(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *model, uint16_t color, uint16_t bg_color)
{
	uint16_t bytes_per_row = (width + 7) / 8;// 每行的字节数：1bit为1个像素，8个像素拼成一个字节，不整除则向上取整
    
    static uint8_t buff[72 * 72 * 2];
    uint8_t *pbuf = buff;
	for (uint16_t row = 0; row < height; row++)
	{
		const uint8_t *row_data = model + row * bytes_per_row;
		for (uint16_t col = 0; col < width; col++)
		{
			uint8_t pixel = row_data[col / 8] & (1 << (7 - col % 8));// col/8判断该列在第几个字节，col%8判断字节的第几位
			// 注意这里是倒序的：取模时约定高位在前，即最左边的像素对应字节的最高位bit7
			// 所以第0列对应bit7，第7列对应bit0
			uint16_t pixel_color = pixel ? color : bg_color;// 根据取模bit判断写16bit的前景色还是背景色
			*pbuf++ = pixel_color & 0xff;// 小端存储，pbuf为uint16_t指针，省去一次CPU字节序转换
			*pbuf++ = (pixel_color >> 8) & 0xff;
		}
	}
    
    st7789_set_range_and_prepare_gram(x, y, x + width - 1, y + height - 1);
    st7789_write_gram(buff, pbuf - buff, false);
}
// 取字模地址
static const uint8_t *ascii_get_model(const char ch, const font_t *font)
{
    uint16_t bytes_per_row = (font->size / 2 + 7) / 8;
    uint16_t bytes_per_char = font->size * bytes_per_row;
    
    if (font->ascii_map)
    {
        const char *map = font->ascii_map;
        do
        {
            if (*map == ch)
            {
                return font->ascii_model + (map - font->ascii_map) * bytes_per_char;
            }
        } while (*(++map) != '\0');
    }
    else
    {
        return font->ascii_model + (ch - ' ') * bytes_per_char;
    }
    
    return NULL;
}
// 写ascii字符
static void st7789_write_ascii(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color, const font_t *font)
{
    if (font == NULL)
        return;
    
    uint16_t fheight = font->size, fwidth = font->size / 2;
    if (!in_screen_range(x, y, x + fwidth - 1, y + fheight - 1))
        return;
    
    if (ch < 0x20 || ch > 0x7E)
        return;
    
	const uint8_t *model = ascii_get_model(ch, font);// 取字模地址，若有映射则从映射查找
    if (model)
        st7789_draw_font(x, y, fwidth, fheight, model, color, bg_color);
}
// 写中文字符
static void st7789_write_chinese(uint16_t x, uint16_t y, const char *ch, uint16_t color, uint16_t bg_color, const font_t *font)
{
    if (ch == NULL || font == NULL)
        return;

    uint16_t fheight = font->size, fwidth = font->size;
    if (!in_screen_range(x, y, x + fwidth - 1, y + fheight - 1))
        return;
    
    const font_chinese_t *c = font->chinese;
    for (; c->name != NULL; c++)
    {
        if (strcmp(c->name, ch) == 0)
            break;
    }
    if (c->name == NULL)
        return;
    
    st7789_draw_font(x, y, fwidth, fheight, c->model, color, bg_color);
}
static bool is_gb2312(char ch)
{
    return ((unsigned char)ch >= 0xA1 && (unsigned char)ch <= 0xF7);
}

void st7789_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font)
{
    while (*str)
    {
        // int len = utf8_char_length(*str);
        int len = is_gb2312(*str) ? 2 : 1;
        if (len <= 0)
        {
            str++;
            continue;
        }
        else if (len == 1)
        {
            st7789_write_ascii(x, y, *str, color, bg_color, font);// 在(x,y)处显示该字符
            str++;// 字符指针后移
            x += font->size / 2;// 横向偏移，即半角宽度
        }
        else
        {
            char ch[5] = {0};
            strncpy(ch, str, len);
            st7789_write_chinese(x, y, ch, color, bg_color, font);
            str += len;
            x += font->size;
        }
    }
}

void st7789_draw_image(uint16_t x, uint16_t y, const image_t *image)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT || 
        x + image->width - 1 >= ST7789_WIDTH || y + image->height - 1 >= ST7789_HEIGHT)
        return;
    
    st7789_set_range_and_prepare_gram(x, y, x + image->width - 1, y + image->height - 1);
    st7789_write_gram((uint8_t *)image->data, image->width * image->height * 2, false);
}
void DMA1_Stream5_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) == SET)
    {
        BaseType_t pxHigherPriorityTaskWoken;
        xSemaphoreGiveFromISR(write_gram_sem, &pxHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
        
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
    }
}
