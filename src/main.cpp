#include <Arduino.h>
#include <neotimer.h>
#include <SPI.h>
#include <Wire.h>
#include <pins.h>
#include <IS32FL3236A.h>
#include <IWatchdog.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FRAM.h>
#include <channel_led.h>
// include all libaries needed

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);
}

volatile uint16_t buffer[6];          // adc buffer for dma
volatile bool adc_new_data_ready = 0; // flag if new adc data is ready

uint16_t vrefanalog = 0;  // analog vcc converted from internal refrence
uint16_t mcu_temp = 0;    // cpu temp
int16_t board_temp = 0;   // board temo in m°C
uint16_t cap_voltage = 0; // capacitor voltage in mV
uint16_t ign_voltage = 0; // ignition node voltage in mV
uint32_t resistance = 0;  // resitance of a channle in mOhm

bool adc_ok = 0;        // flag if adc is starded sucsefully
bool led_driver_ok = 0; // flag if led driver is okay
bool fram_ok = 0;       // flag if fram is okay
bool oled_ok = 0;       // flag if oled is okay

uint8_t channel_pins[16] = {CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};
uint32_t channel_res[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t channel_res_avg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t channel_counter_res = 0;
uint16_t avg_counter_res = 0;
uint8_t channel_ignitbale[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t channel_pop_unused[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool channel_needed[16] = {1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1};
bool led_blink_helper = 0;

unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;
bool once = 0;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TwoWire sensor_i2c(SENSOR_SDA, SENSOR_SCL);
TwoWire oled_i2c(OLED_SDA, OLED_SCL);
IS32FL3236A channel_leds(LED_DRIVER_ADDRESS, SDB, &sensor_i2c);
Adafruit_SSD1306 display(128, 64, &oled_i2c, -1);
HardwareSerial RS485_1(RS485_1_RX, RS485_1_TX);
HardwareSerial RS485_2(RS485_2_RX, RS485_2_TX);
HardwareSerial BAT_SERIAL(BATTERY_RX, BATTERY_TX);
HardwareSerial EXP_SERIAL(EXPANSION_RX, EXPANSION_TX);
FRAM9 fram;
IS32FL3236A led_driver(LED_DRIVER_ADDRESS, SDB, &sensor_i2c);
channel_led leds16(&led_driver);
Neotimer resistance_timer(6);
Neotimer led_blink_timer(250);

static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
void convertADC();
int16_t adcToTemperature(uint16_t adcValue);
void setup_gpio();
void setup_pheripherals();
void calculate_resistance();
void check_ignitable();
void setChannelLEDsRes();
void blinkLEDsRes();

void setup()
{
  IWatchdog.begin(8000000); // WATCHDOG TIMER IS STILL TO LONG NEED TO BE CHANGED
  IWatchdog.reload();

  setup_gpio();
  setup_pheripherals();
}

void loop()
{
  IWatchdog.reload();
  calculate_resistance();
}

void convertADC()
{
  if (adc_new_data_ready == 1)
  {
    vrefanalog = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(buffer[0], ADC_RESOLUTION_12B);
    mcu_temp = __HAL_ADC_CALC_TEMPERATURE(vrefanalog, buffer[5], ADC_RESOLUTION_12B);
    board_temp = adcToTemperature(buffer[4]);
    cap_voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[3], ADC_RESOLUTION_12B) * 11;
    ign_voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[1], ADC_RESOLUTION_12B) * 11;
    resistance = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[2], ADC_RESOLUTION_12B) / 2 * 100;
    adc_new_data_ready = 0;
  }
}
void setup_gpio()
{
  pinMode(TEST_LED, OUTPUT);
  pinMode(CAP_SW, OUTPUT);
  pinMode(IGN_SW1, OUTPUT);
  pinMode(IGN_SW2, OUTPUT);
  pinMode(CAP_DIS, OUTPUT);

  pinMode(CH1, OUTPUT);
  pinMode(CH2, OUTPUT);
  pinMode(CH3, OUTPUT);
  pinMode(CH4, OUTPUT);
  pinMode(CH5, OUTPUT);
  pinMode(CH6, OUTPUT);
  pinMode(CH7, OUTPUT);
  pinMode(CH8, OUTPUT);
  pinMode(CH9, OUTPUT);
  pinMode(CH10, OUTPUT);
  pinMode(CH11, OUTPUT);
  pinMode(CH12, OUTPUT);
  pinMode(CH13, OUTPUT);
  pinMode(CH14, OUTPUT);
  pinMode(CH15, OUTPUT);
  pinMode(CH16, OUTPUT);

  pinMode(RS485_1_DIR, OUTPUT);
  pinMode(RS485_2_DIR, OUTPUT);

  pinMode(OE_LVL, OUTPUT);
  pinMode(NRST_BAT, OUTPUT);
  pinMode(BOOT0_BAT, OUTPUT);

  pinMode(NRF24_ON, OUTPUT);
  pinMode(NRF24_FLG, INPUT);

  digitalWrite(CAP_SW, LOW);
  digitalWrite(IGN_SW1, LOW);
  digitalWrite(IGN_SW2, LOW);
  digitalWrite(CAP_DIS, LOW);

  digitalWrite(CH1, LOW);
  digitalWrite(CH2, LOW);
  digitalWrite(CH3, LOW);
  digitalWrite(CH4, LOW);
  digitalWrite(CH5, LOW);
  digitalWrite(CH6, LOW);
  digitalWrite(CH7, LOW);
  digitalWrite(CH8, LOW);
  digitalWrite(CH9, LOW);
  digitalWrite(CH10, LOW);
  digitalWrite(CH11, LOW);
  digitalWrite(CH12, LOW);
  digitalWrite(CH13, LOW);
  digitalWrite(CH14, LOW);
  digitalWrite(CH15, LOW);
  digitalWrite(CH16, LOW);

  digitalWrite(RS485_1_DIR, LOW);
  digitalWrite(RS485_2_DIR, LOW);

  digitalWrite(OE_LVL, LOW);
  digitalWrite(NRST_BAT, HIGH);
  digitalWrite(BOOT0_BAT, LOW);

  digitalWrite(NRF24_ON, LOW);
}
void setup_pheripherals()
{
  SerialUSB.begin(115200);
  RS485_1.begin(19200);
  RS485_2.begin(19200);
  BAT_SERIAL.begin(38400);
  EXP_SERIAL.begin(38400);

  if (leds16.begin() == 0)
  {
    leds16.setLedBrightness(1.00);
    leds16.clearAll();
    leds16.setSleep(0);
    led_driver_ok = 1;
  }

  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS) == 1)
  {
    display.clearDisplay();
    display.display();
    oled_ok = 1;
  }
  else
  {
    oled_ok = 0;
  }

  if (fram.begin(FRAM_ADDRESS, FRAM_WP) == 0)
  {
    fram_ok = 1;
  }
  else
  {
    fram_ok = 0;
  }

  MX_DMA_Init();
  MX_ADC1_Init();

  if (HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK)
  {
    adc_ok = 1;
  }
  else
  {
    adc_ok = 0;
  }
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)buffer, 6) == HAL_OK)
  {
    adc_ok = 1;
  }
  else
  {
    adc_ok = 0;
  }
}
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV16;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 6;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_79CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_79CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}
extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if (hadc->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */

    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA1     ------> ADC1_IN1
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    PB2     ------> ADC1_IN10
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */
  }
}
extern "C" void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA1     ------> ADC1_IN1
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    PB2     ------> ADC1_IN10
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }
}
extern "C" void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  adc_new_data_ready = 1;
}
int16_t adcToTemperature(uint16_t adcValue)
{
  // calculate the resistance of the NTC thermistor
  float resistance = 10000 * ((float)adcValue / (4096 - adcValue));

  // calculate the temperature in Kelvin using the Beta parameter equation
  float temperatureK = 1 / (log(resistance / 10000) / 3950 + 1 / 298.15); // 298.15K is 25°C

  // convert the temperature from Kelvin to Celsius
  float temperatureC = temperatureK - 273.15;

  // round the temperature to the nearest 0.5°C
  float roundedTemperatureC = round(temperatureC * 2) / 2.0;

  // convert the temperature to an int16_t
  int16_t temperatureInt = (int16_t)(roundedTemperatureC * 10); // multiply by 10 to preserve the .5 precision

  return temperatureInt; // return value
}
void calculate_resistance()
{
  if (resistance_timer.repeat())
  {
    if (channel_counter_res < 16)
    {
      digitalWrite(channel_pins[channel_counter_res], HIGH);
      convertADC();
      if (avg_counter_res < 6)
      {
        if (avg_counter_res > 0)
        {
          channel_res_avg[channel_counter_res] = resistance + channel_res_avg[channel_counter_res];
        }
        avg_counter_res++;
      }
      else
      {
        avg_counter_res = 0;
        digitalWrite(channel_pins[channel_counter_res], LOW);
        channel_counter_res++;
      }
    }
    else
    {
      channel_counter_res = 0;
      for (int i = 0; i < 16; i++)
      {
        channel_res[i] = channel_res_avg[i] / 5;
        channel_res_avg[i] = 0;
      }
      check_ignitable();
      setChannelLEDsRes();
      blinkLEDsRes();
    }
  }
}
void check_ignitable()
{
  for (int i = 0; i < 16; i++)
  {
    if (channel_res[i] < 14000 && channel_res[i] > 125)
    {
      channel_ignitbale[i] = 1;
    }
    else
    {
      channel_ignitbale[i] = 0;
    }
    if (channel_res[i] > 100000)
    {
      channel_ignitbale[i] = 2;
    }
  }
}
void setChannelLEDsRes()
{
  if (led_driver_ok == 1)
  {
    for (int i = 0; i < 16; i++)
    {
      if (channel_needed[i] == 1)
      {
        switch (channel_ignitbale[i])
        {
        case 0:
          leds16.setLEDState(i, LED_RED);
          break;
        case 1:
          leds16.setLEDState(i, LED_GREEN);
          break;
        case 2:
          leds16.setLEDState(i, LED_YELLOW);
          break;
        default:
          leds16.setLEDState(i, LED_OFF);
          break;
        }
        channel_pop_unused[i] = 3;
      }
      else
      {
        channel_pop_unused[i] = channel_ignitbale[i];
      }
    }
  }
}
void blinkLEDsRes()
{
  led_blink_helper = !led_blink_helper;

  for (int i = 0; i < 16; i++)
  {
    if (led_blink_helper == 0)
    {
      if (channel_pop_unused[i] == 0 || channel_pop_unused[i] == 1)
      {
        leds16.setLEDState(i, LED_OFF);
      }
    }
    if (led_blink_helper == 1)
    {
      if (channel_pop_unused[i] == 0)
      {
        leds16.setLEDState(i, LED_RED);
      }
      if (channel_pop_unused[i] == 1)
      {
        leds16.setLEDState(i, LED_GREEN);
      }
    }
    if (channel_pop_unused[i] == 2)
    {
      leds16.setLEDState(i, LED_OFF);
    }
  }
}