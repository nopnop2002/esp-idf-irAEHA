#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "driver/rmt.h"
#include "ir_tools.h"

#if defined(M5STACK)
#define CONFIG_STACK 1
#elif defined(M5STICK)
#define CONFIG_STICK 1
#elif defined(M5STICK_C)
#define CONFIG_STICKC	1
#elif defined(M5STICK_C_PLUS)
#define CONFIG_STICKC_PLUS 1
#endif

#if CONFIG_STACK
#include "ili9340.h"
#include "fontx.h"
#endif

#if CONFIG_STICK
#include "sh1107.h"
#include "font8x8_basic.h"
#endif

#if CONFIG_STICKC
#include "axp192.h"
#include "st7735s.h"
#include "fontx.h"
#endif

#if CONFIG_STICKC_PLUS
#include "axp192.h"
#include "st7789.h"
#include "fontx.h"
#endif

#if CONFIG_STACK
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CS_GPIO 14
#define DC_GPIO 27
#define RESET_GPIO 33
#define BL_GPIO 32
#define FONT_WIDTH 12
#define FONT_HEIGHT 24
#define MAX_CONFIG 20
#define MAX_LINE 8
#define MAX_CHARACTER 26
#define GPIO_INPUT_A GPIO_NUM_39
#define GPIO_INPUT_B GPIO_NUM_38
#define GPIO_INPUT_C GPIO_NUM_37
#define RMT_TX_CHANNEL 1 /*!< RMT channel for transmitter */
// GROVE PORT A
#define RMT_TX_GPIO_NUM	GPIO_NUM_21 /*!< GPIO number for transmitter signal */
// GROVE PORT B
//#define RMT_TX_GPIO_NUM	GPIO_NUM_26 /*!< GPIO number for transmitter signal */
// GROVE PORT C
//#define RMT_TX_GPIO_NUM	GPIO_NUM_17 /*!< GPIO number for transmitter signal */
#endif

#if CONFIG_STICK
#define MAX_CONFIG 14
#define MAX_LINE 14
#define MAX_CHARACTER 8
#define GPIO_INPUT GPIO_NUM_35
#define GPIO_BUZZER GPIO_NUM_26
#define RMT_TX_CHANNEL 1 /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM	GPIO_NUM_17 /*!< GPIO number for transmitter signal */
#endif

#if CONFIG_STICKC
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 160
#define OFFSET_X 26
#define OFFSET_Y 1
#define GPIO_MOSI 15
#define GPIO_SCLK 13
#define GPIO_CS 5
#define GPIO_DC 23
#define GPIO_RESET 18
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define MAX_CONFIG 20
#define MAX_LINE 8
#define MAX_CHARACTER 10
#define GPIO_INPUT_A GPIO_NUM_37
#define GPIO_INPUT_B GPIO_NUM_39
#define RMT_TX_CHANNEL 1 /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM	GPIO_NUM_9 /*!< GPIO number for transmitter signal */
#endif

#if CONFIG_STICKC_PLUS
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define OFFSETX 52
#define OFFSETY 40
#define GPIO_MOSI 15
#define GPIO_SCLK 13
#define GPIO_CS 5
#define GPIO_DC 23
#define GPIO_RESET 18
#define GPIO_BL -1
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define MAX_CONFIG 20
#define MAX_LINE 12
#define MAX_CHARACTER 16
#define GPIO_INPUT_A GPIO_NUM_37
#define GPIO_INPUT_B GPIO_NUM_39
#define RMT_TX_CHANNEL 1 /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM	GPIO_NUM_9 /*!< GPIO number for transmitter signal */
#endif


#define CMD_UP 100
#define CMD_DOWN 200
#define CMD_TOP 300
#define CMD_BOTTOM 400
#define CMD_SELECT 500

QueueHandle_t xQueueCmd;

static const char *TAG = "M5Remote";

typedef struct {
	uint16_t command;
	TaskHandle_t taskHandle;
} CMD_t;

typedef struct {
	bool enable;
	char display_text[MAX_CHARACTER+1];
	int8_t ir_fire_counter;
	uint8_t ir_data_num;
	uint8_t ir_data[20];
} DISPLAY_t;


static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(TAG,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

#if CONFIG_STICK
void buttonStick(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT);
	gpio_set_direction(GPIO_INPUT, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(0),"diffTick=%"PRIu32,diffTick);
			cmdBuf.command = CMD_DOWN;
			if (diffTick > 100) cmdBuf.command = CMD_SELECT;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif

#if CONFIG_STICKC || CONFIG_STICKC_PLUS || CONFIG_STACK
void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_SELECT;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
				if (level == 1) break;
				vTaskDelay(1);
			}
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif

#if CONFIG_STICKC || CONFIG_STICKC_PLUS
void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(0),"diffTick=%"PRIu32,diffTick);
			cmdBuf.command = CMD_DOWN;
			if (diffTick > 100) cmdBuf.command = CMD_TOP;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_STICKC || CONFIG_STICKC_PLUS

#if CONFIG_STACK
void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(0),"diffTick=%"PRIu32,diffTick);
			cmdBuf.command = CMD_DOWN;
			if (diffTick > 100) cmdBuf.command = CMD_BOTTOM;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_STACK

#if CONFIG_STACK
void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_C);
	gpio_set_direction(GPIO_INPUT_C, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_C);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_C);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(0),"diffTick=%"PRIu32,diffTick);
			cmdBuf.command = CMD_UP;
			if (diffTick > 100) cmdBuf.command = CMD_TOP;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_STACK

static int parseLine(char *line, int size1, int size2, char arr[size1][size2])
{
	ESP_LOGD(TAG, "line=[%s]", line);
	int dst = 0;
	int pos = 0;
	int llen = strlen(line);
	bool inq = false;

	for(int src=0;src<llen;src++) {
		char c = line[src];
		ESP_LOGD(TAG, "src=%d c=%c", src, c);
		if (c == ',') {
			if (inq) {
				if (pos == (size2-1)) continue;
				arr[dst][pos++] = line[src];
				arr[dst][pos] = 0;
			} else {
				ESP_LOGD(TAG, "arr[%d]=[%s]",dst,arr[dst]);
				dst++;
				if (dst == size1) break;
				pos = 0;
			}

		} else if (c == ';') {
			if (inq) {
				if (pos == (size2-1)) continue;
				arr[dst][pos++] = line[src];
				arr[dst][pos] = 0;
			} else {
				ESP_LOGD(TAG, "arr[%d]=[%s]",dst,arr[dst]);
				dst++;
				break;
			}

		} else if (c == '"') {
			inq = !inq;

		} else if (c == '\'') {
			inq = !inq;

		} else {
			if (pos == (size2-1)) continue;
			arr[dst][pos++] = line[src];
			arr[dst][pos] = 0;
		}
	}

	return dst;
}


static int readDefineFile(DISPLAY_t *display, size_t maxLine, size_t maxText) {
	int readLine = 0;
	ESP_LOGI(pcTaskGetName(0), "Reading file:maxText=%d",maxText);
	FILE* f = fopen("/spiffs/Display.def", "r");
	if (f == NULL) {
			ESP_LOGE(pcTaskGetName(0), "Failed to open define file for reading");
			ESP_LOGE(pcTaskGetName(0), "Please make Display.def");
			return 0;
	}
	char line[64];
	char result[10][32];
	while (1){
		if ( fgets(line, sizeof(line) ,f) == 0 ) break;
		// strip newline
		char* pos = strchr(line, '\n');
		if (pos) {
			*pos = '\0';
		}
		ESP_LOGD(pcTaskGetName(0), "line=[%s]", line);
		if (strlen(line) == 0) continue;
		if (line[0] == '#') continue;

		int ret = parseLine(line, 10, 32, result);
		ESP_LOGI(TAG, "parseLine=%d", ret);
		for(int i=0;i<ret;i++) ESP_LOGD(TAG, "result[%d]=[%s]", i, &result[i][0]);
		display[readLine].enable = true;
		//strlcpy(display[readLine].display_text, &result[0][0], maxText);
		strlcpy(display[readLine].display_text, &result[0][0], maxText+1);
		display[readLine].ir_fire_counter= strtol(&result[1][0], NULL, 10);
		display[readLine].ir_data_num = ret-2;
		for(int i=2;i<ret;i++) {
			display[readLine].ir_data[i-2] = strtol(&result[i][0], NULL, 16);
		}

		readLine++;
		if (readLine == maxLine) break;
	}
	fclose(f);
	return readLine;
}

#if CONFIG_STICKC || CONFIG_STICKC_PLUS || CONFIG_STACK
void tft(void *pvParameters)
{
	// set font file
#if CONFIG_STICKC || CONFIG_STICKC_PLUS
	FontxFile fxG[2];
	InitFontx(fxG,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
	FontxFile fxM[2];
	InitFontx(fxM,"/spiffs/ILMH16XB.FNT",""); // 8x16Dot Mincyo
#endif

#if CONFIG_STACK
	FontxFile fxG[2];
	InitFontx(fxG,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
	FontxFile fxM[2];
	InitFontx(fxM,"/spiffs/ILMH24XB.FNT",""); // 12x24Dot Mincyo
#endif

	// Setup IR transmitter
	rmt_item32_t *items = NULL;
	size_t length = 0;
	ir_builder_t *ir_builder = NULL;
	rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX(RMT_TX_GPIO_NUM, RMT_TX_CHANNEL);
	rmt_tx_config.tx_config.carrier_en = true;
	rmt_config(&rmt_tx_config);
	rmt_driver_install(RMT_TX_CHANNEL, 0, 0);
	ir_builder_config_t ir_builder_config = IR_BUILDER_DEFAULT_CONFIG((ir_dev_t)RMT_TX_CHANNEL);
	ir_builder_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT;
	ir_builder = ir_builder_rmt_new_aeha(&ir_builder_config);
	ESP_LOGI(pcTaskGetName(0), "Setup IR transmitter done");

	// Setup Screen
#if CONFIG_STICKC
	ST7735_t dev;
	spi_master_init(&dev, GPIO_MOSI, GPIO_SCLK, GPIO_CS, GPIO_DC, GPIO_RESET);
	lcdInit(&dev, SCREEN_WIDTH, SCREEN_HEIGHT, OFFSET_X, OFFSET_Y);
#endif

#if CONFIG_STICKC_PLUS
	TFT_t dev;
	spi_master_init(&dev, GPIO_MOSI, GPIO_SCLK, GPIO_CS, GPIO_DC, GPIO_RESET, GPIO_BL);
	lcdInit(&dev, SCREEN_WIDTH, SCREEN_HEIGHT, OFFSETX, OFFSETY);
#endif

#if CONFIG_STACK
	TFT_t dev;
	spi_master_init(&dev, CS_GPIO, DC_GPIO, RESET_GPIO, BL_GPIO);
	lcdInit(&dev, 0x9341, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
#endif
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	// Read display information
	DISPLAY_t display[MAX_CONFIG];
	for(int i=0;i<MAX_CONFIG;i++) display[i].enable = false;
	int readLine = readDefineFile(display, MAX_CONFIG, MAX_CHARACTER);
	ESP_LOGI(pcTaskGetName(0), "readLine=%d",readLine);
	if (readLine == 0) {
		while(1) { vTaskDelay(1); }
	}
	for(int i=0;i<readLine;i++) {
		ESP_LOGI(pcTaskGetName(0), "display[%d].display_text=[%s]",i, display[i].display_text);
		ESP_LOGI(pcTaskGetName(0), "display[%d].ir_fire_counter=[%d]",i, display[i].ir_fire_counter);
		ESP_LOGI(pcTaskGetName(0), "display[%d].ir_data_num=[%d]",i, display[i].ir_data_num);
		for(int j=0;j<display[i].ir_data_num;j++) {
			ESP_LOGI(pcTaskGetName(0), "display[%d].ir_data[%d]=[0x%02x]",i, j, display[i].ir_data[j]);
		}
	}

	// Initial Screen
	uint16_t color;
	uint8_t ascii[MAX_CHARACTER+1];
	uint16_t ypos;
	lcdFillScreen(&dev, BLACK);
	color = RED;
	lcdSetFontDirection(&dev, 0);
	ypos = FONT_HEIGHT-1;
#if CONFIG_STICKC
	strcpy((char *)ascii, "M5 StickC");
#endif
#if CONFIG_STICKC_PLUS
	strcpy((char *)ascii, "M5 StickC+");
#endif
#if CONFIG_STACK
	strcpy((char *)ascii, "M5 Stack");
#endif
	lcdDrawString(&dev, fxG, 0, ypos, ascii, color);

	int offset = 0;
	for(int i=0;i<MAX_LINE;i++) {
		ypos = FONT_HEIGHT * (i+3) - 1;
		ascii[0] = 0;
		if (display[i+offset].enable) strcpy((char *)ascii, display[i+offset].display_text);
		if (i == 0) {
			lcdDrawString(&dev, fxG, 0, ypos, ascii, YELLOW);
		} else {
			lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);
		}
	}

	int selected = 0;
	CMD_t cmdBuf;
	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(0),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_DOWN) {
			ESP_LOGI(pcTaskGetName(0), "selected=%d offset=%d readLine=%d",selected, offset, readLine);
			if ((selected+offset+1) == readLine) continue;

			ypos = FONT_HEIGHT * (selected+3) - 1;
			strcpy((char *)ascii, display[selected+offset].display_text);
			lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);

			// Scroll Down
			if (selected+1 == MAX_LINE) {
				lcdDrawFillRect(&dev, 0, FONT_HEIGHT-1, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
				offset++;
				for(int i=0;i<MAX_LINE;i++) {
					ypos = FONT_HEIGHT * (i+3) - 1;
					ascii[0] = 0;
					if (display[i+offset].enable) strcpy((char *)ascii, display[i+offset].display_text);
					lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);
				}
			} else {
				selected++;
			}
			ESP_LOGI(pcTaskGetName(0), "selected=%d offset=%d",selected, offset);

			ypos = FONT_HEIGHT * (selected+3) - 1;
			strcpy((char *)ascii, display[selected+offset].display_text);
			//lcdDrawString(&dev, fxG, 0, ypos, ascii, BLACK);
			lcdDrawString(&dev, fxG, 0, ypos, ascii, YELLOW);

		} else if (cmdBuf.command == CMD_UP) {
			ESP_LOGI(pcTaskGetName(0), "selected=%d offset=%d",selected, offset);
			if (selected+offset == 0) continue;

			ypos = FONT_HEIGHT * (selected+3) - 1;
			strcpy((char *)ascii, display[selected+offset].display_text);
			lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);

			// Scroll Up
			if (offset > 0) {
				lcdDrawFillRect(&dev, 0, FONT_HEIGHT-1, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
				offset--;
				for(int i=0;i<MAX_LINE;i++) {
					ypos = FONT_HEIGHT * (i+3) - 1;
					ascii[0] = 0;
					if (display[i+offset].enable) strcpy((char *)ascii, display[i+offset].display_text);
					lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);
				}
			} else {
				selected--;
			}
			ypos = FONT_HEIGHT * (selected+3) - 1;
			strcpy((char *)ascii, display[selected+offset].display_text);
			//lcdDrawString(&dev, fxG, 0, ypos, ascii, BLACK);
			lcdDrawString(&dev, fxG, 0, ypos, ascii, YELLOW);

		} else if (cmdBuf.command == CMD_TOP) {
			offset = 0;
			selected = 0;
			lcdDrawFillRect(&dev, 0, FONT_HEIGHT-1, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
			for(int i=0;i<MAX_LINE;i++) {
				ypos = FONT_HEIGHT * (i+3) - 1;
				ascii[0] = 0;
				if (display[i+offset].enable) strcpy((char *)ascii, display[i+offset].display_text);
				if (i == 0) {
					lcdDrawString(&dev, fxG, 0, ypos, ascii, YELLOW);
				} else {
					lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);
				}
			}

		} else if (cmdBuf.command == CMD_BOTTOM) {
			ESP_LOGI(pcTaskGetName(0), "readLine=%d MAX_LINE=%d",readLine, MAX_LINE );
			offset = 0;
			selected = readLine-1;
			if (readLine > MAX_LINE) {
				offset = readLine - MAX_LINE;
				selected = MAX_LINE - 1;
			}
			ESP_LOGI(pcTaskGetName(0), "selected=%d offset=%d",selected, offset);
			lcdDrawFillRect(&dev, 0, FONT_HEIGHT-1, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
			for(int i=0;i<MAX_LINE;i++) {
				ypos = FONT_HEIGHT * (i+3) - 1;
				ascii[0] = 0;
				if (display[i+offset].enable) strcpy((char *)ascii, display[i+offset].display_text);
				//if (i == 0) {
				if (i+offset+1 == readLine) {
					lcdDrawString(&dev, fxG, 0, ypos, ascii, YELLOW);
				} else {
					lcdDrawString(&dev, fxG, 0, ypos, ascii, CYAN);
				}
			}

		} else if (cmdBuf.command == CMD_SELECT) {
			ESP_LOGI(pcTaskGetName(0), "selected=%d offset=%d",selected, offset);
			ESP_LOGI(pcTaskGetName(0), "ir_fire_counter=%d",display[selected+offset].ir_fire_counter);
			ESP_LOGI(pcTaskGetName(0), "ir_data_num=%d",display[selected+offset].ir_data_num);
			for (int i=0;i<display[selected+offset].ir_data_num;i++) {
				ESP_LOGI(pcTaskGetName(0), "ir_data[%d]=%02x",i, display[selected+offset].ir_data[i]);
			}
			// Send new key code
			ESP_ERROR_CHECK(ir_builder->build_frame(ir_builder, display[selected+offset].ir_data_num, display[selected+offset].ir_data));
			ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
			//To send data according to the waveform items.
			for (int i=0;i<display[selected+offset].ir_fire_counter;i++) {
				rmt_write_items(RMT_TX_CHANNEL, items, length, false);
			}
		}
	} // end while

	// nerver reach here
	ir_builder->del(ir_builder);
	rmt_driver_uninstall(RMT_TX_CHANNEL);
	vTaskDelete(NULL);
}
#endif // CONFIG_STICKC || CONFIG_STICKC_PLUS || CONFIG_STACK


#if CONFIG_STICK
void buzzerON(void) {
	gpio_reset_pin( GPIO_BUZZER );
	gpio_set_direction( GPIO_BUZZER, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_BUZZER, 0 );
	for(int i=0;i<50;i++){
		gpio_set_level( GPIO_BUZZER, 1 );
		vTaskDelay(1);
		gpio_set_level( GPIO_BUZZER, 0 );
		vTaskDelay(1);
	}
}


void tft(void *pvParameters)
{
	// Setup IR transmitter
	rmt_item32_t *items = NULL;
	size_t length = 0;
	ir_builder_t *ir_builder = NULL;
	rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX(RMT_TX_GPIO_NUM, RMT_TX_CHANNEL);
	rmt_tx_config.tx_config.carrier_en = true;
	rmt_config(&rmt_tx_config);
	rmt_driver_install(RMT_TX_CHANNEL, 0, 0);
	ir_builder_config_t ir_builder_config = IR_BUILDER_DEFAULT_CONFIG((ir_dev_t)RMT_TX_CHANNEL);
	ir_builder_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT;
	ir_builder = ir_builder_rmt_new_aeha(&ir_builder_config);
	ESP_LOGI(pcTaskGetName(0), "Setup IR transmitter done");

	// Setup Screen
	SH1107_t dev;
	spi_master_init(&dev);
	spi_init(&dev, 64, 128);
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	// Read display information
	DISPLAY_t display[MAX_CONFIG];
	for(int i=0;i<MAX_CONFIG;i++) display[i].enable = false;
	int readLine = readDefineFile(display, MAX_CONFIG, MAX_CHARACTER);
	if (readLine == 0) {
		while(1) { vTaskDelay(1); }
	}
	for(int i=0;i<readLine;i++) {
		ESP_LOGI(pcTaskGetName(0), "display[%d].display_text=[%s]",i, display[i].display_text);
		ESP_LOGI(pcTaskGetName(0), "display[%d].ir_fire_counter=[%d]",i, display[i].ir_fire_counter);
		ESP_LOGI(pcTaskGetName(0), "display[%d].ir_data_num=[%d]",i, display[i].ir_data_num);
		for(int j=0;j<display[i].ir_data_num;j++) {
			ESP_LOGI(pcTaskGetName(0), "display[%d].ir_data[%d]=[0x%02x]",i, j, display[i].ir_data[j]);
		}
	}

	// Initial Screen
	clear_screen(&dev, false);
	display_contrast(&dev, 0xff);
	char ascii[MAX_CHARACTER+1];
	uint16_t ypos;
	strcpy(ascii, "M5 Stick");
	display_text(&dev, 0, ascii, 8, false);

	for(int i=0;i<MAX_LINE;i++) {
		ypos = i + 2;
		ascii[0] = 0;
		if (display[i].enable) strcpy(ascii, display[i].display_text);
		if (i == 0) {
			display_text(&dev, ypos, ascii, strlen(ascii), true);
		} else {
			display_text(&dev, ypos, ascii, strlen(ascii), false);
		}
	} // end for

	int selected = 0;
	CMD_t cmdBuf;
	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(0),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_DOWN) {
			strcpy(ascii, display[selected].display_text);
			ypos = selected + 2;
			display_text(&dev, ypos, ascii, strlen(ascii), false);

			selected++;
			if (selected == readLine) selected = 0;
			strcpy(ascii, display[selected].display_text);
			ypos = selected + 2;
			display_text(&dev, ypos, ascii, strlen(ascii), true);

		} else if (cmdBuf.command == CMD_SELECT) {
			ESP_LOGI(pcTaskGetName(0), "selected=%d",selected);
			ESP_LOGI(pcTaskGetName(0), "ir_fire_counter=%d",display[selected].ir_fire_counter);
			ESP_LOGI(pcTaskGetName(0), "ir_data_num=%d",display[selected].ir_data_num);
			for (int i=0;i<display[selected].ir_data_num;i++) {
				ESP_LOGI(pcTaskGetName(0), "ir_data[%d]=%02x",i, display[selected].ir_data[i]);
			}
			// Send new key code
			ESP_ERROR_CHECK(ir_builder->build_frame(ir_builder, display[selected].ir_data_num, display[selected].ir_data));
			ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
			//To send data according to the waveform items.
			for (int i=0;i<display[selected].ir_fire_counter;i++) {
				rmt_write_items(RMT_TX_CHANNEL, items, length, false);
			}
		}
	} // end while

	// nerver reach here
	ir_builder->del(ir_builder);
	rmt_driver_uninstall(RMT_TX_CHANNEL);
	vTaskDelete(NULL);
}
#endif // CONFIG_STICK

void app_main(void)
{
	ESP_LOGI(TAG, "Initializing SPIFFS");
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 10,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret =esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret ==ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret== ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	SPIFFS_Directory("/spiffs");

	/* Create Queue */
	xQueueCmd = xQueueCreate( 10, sizeof(CMD_t) );
	configASSERT( xQueueCmd );

#if CONFIG_STICKC
	// power on
	i2c_master_init();
	AXP192_PowerOn();
#endif

#if CONFIG_STICKC_PLUS
	// power on
	i2c_master_init();
	AXP192_PowerOn();
	AXP192_ScreenBreath(11);
#endif

	xTaskCreate(tft, "TFT", 1024*4, NULL, 2, NULL);

#if CONFIG_STACK
	xTaskCreate(buttonA, "SELECT", 1024*4, NULL, 2, NULL);
	xTaskCreate(buttonB, "DOWN", 1024*4, NULL, 2, NULL);
	xTaskCreate(buttonC, "UP", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_STICKC || CONFIG_STICKC_PLUS
	xTaskCreate(buttonA, "SELECT", 1024*4, NULL, 2, NULL);
	xTaskCreate(buttonB, "DOWN", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_STICK
	xTaskCreate(buttonStick, "BUTTON", 1024*4, NULL, 2, NULL);
#endif
}

