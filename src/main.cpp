#include <Arduino.h>
#include <neotimer.h>
#include <SPI.h>
#include <Wire.h>
#include <pins.h>
#include <IS32FL3236A.h>
#include <IWatchdog.h>

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

volatile uint16_t buffer[6];          // ADC BUFFER
volatile bool adc_new_data_ready = 0; // ADC DATA READY
volatile bool convert_adc = 0;        // Convert ADC every 500mS

uint16_t vrefanalog = 0;  // ANALOG VREF VOLTAGE IN mV
uint16_t mcu_temp = 0;    // CPU TEMP
int16_t board_temp = 0;   // BOARD TEMP IN MILIDEGREE
uint16_t cap_voltage = 0; // CAP VOLTAGE IN MILIDEGREE
uint16_t ign_voltage = 0; // CAP VOLTAGE IN MILIDEGREE
uint16_t resistance = 0;  // RESISTANCE CALULATED ROM MESUREMNT CIRCUIT IN MILIOHM

int test = 0;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
Neotimer adcTimer = Neotimer();
Neotimer uartTimer = Neotimer();
TwoWire sensor_i2c(SENSOR_SDA, SENSOR_SCL);
TwoWire oled_i2c(SENSOR_SDA, SENSOR_SCL);
IS32FL3236A channel_leds(0x3C, SDB, &sensor_i2c);

void timer_10ms();
void timer_500ms();
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
void convertADC();
int16_t adcToTemperature(uint16_t adcValue);
void togglePin(char input);

void setup()
{
  IWatchdog.begin(8000000); // WATCHDOG TIMER IS STILL TO LONG NEED TO BE CHANGED

  pinMode(TEST_LED, OUTPUT);
  pinMode(CAP_SW, OUTPUT);
  pinMode(IGN_SW1, OUTPUT);
  pinMode(IGN_SW2, OUTPUT);
  pinMode(CAP_DIS, OUTPUT);

  pinMode(CH1, OUTPUT);

  digitalWrite(CAP_SW, LOW);
  digitalWrite(IGN_SW1, LOW);
  digitalWrite(IGN_SW2, LOW);
  digitalWrite(CAP_DIS, LOW);

  digitalWrite(CH1, LOW);

  SerialUSB.begin(115200);

  channel_leds.begin();
  channel_leds.sleep(0);
  channel_leds.setFrequency(1);
  channel_leds.clear();
  channel_leds.update();

  for (int i = 0; i < 32; i++)
  {
    channel_leds.setLedParam(i, IS32FL3236A_IMAX, 1);
    channel_leds.setLedPwm(i, 0);
  }
  channel_leds.update();

  MX_DMA_Init();
  MX_ADC1_Init();

  if (HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK)
  {
    SerialUSB.println("*CALOK074*");
  }
  else
  {
    SerialUSB.println("*CALF008");
  }
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)buffer, 6) == HAL_OK)
  {
    SerialUSB.println("*CONVOK016*");
  }
  else
  {
    SerialUSB.println("*CONVF082*");
  }

  adcTimer.set(10);
  uartTimer.set(500);
}

void loop()
{
  timer_10ms();
  timer_500ms();
  convertADC();
  IWatchdog.reload();

  // Check for SerialUSB input
  if (SerialUSB.available() > 0)
  {
    char input = SerialUSB.read();
    togglePin(input);
  }
}

void togglePin(char input)
{
  switch (input)
  {
  case '1':
    digitalToggle(CAP_SW);
    break;
  case '2':
    digitalToggle(IGN_SW1);
    break;
  case '3':
    digitalToggle(IGN_SW2);
    break;
  case '4':
    digitalToggle(CAP_DIS);
    break;
  default:
    SerialUSB.println("Invalid input. Please enter a number between 1 and 4.");
    break;
  }
}

void convertADC()
{
  if (adc_new_data_ready == 1 && convert_adc == 1)
  {
    vrefanalog = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(buffer[0], ADC_RESOLUTION_12B);
    mcu_temp = __HAL_ADC_CALC_TEMPERATURE(vrefanalog, buffer[5], ADC_RESOLUTION_12B);
    board_temp = adcToTemperature(buffer[4]);
    cap_voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[3], ADC_RESOLUTION_12B) * 11;
    ign_voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[1], ADC_RESOLUTION_12B) * 11;
    resistance = __HAL_ADC_CALC_DATA_TO_VOLTAGE(vrefanalog, buffer[2], ADC_RESOLUTION_12B) / 2;
    adc_new_data_ready = 0;
  }
}

void timer_10ms()
{
  if (adcTimer.repeat())
  {
    convert_adc = 1;
  }
}
void timer_500ms()
{
  if (uartTimer.repeat())
  {
    SerialUSB.println("*********************************");
    SerialUSB.print("VREF:");
    SerialUSB.println(vrefanalog);
    SerialUSB.print("MCU TEMP:");
    SerialUSB.println(mcu_temp);
    SerialUSB.print("BOARD TEMP:");
    SerialUSB.println(board_temp);
    SerialUSB.print("CAP VOLTAGE:");
    SerialUSB.println(cap_voltage);
    SerialUSB.print("IGN VOLTAGE:");
    SerialUSB.println(ign_voltage);
    SerialUSB.print("RESISTANCE:");
    SerialUSB.println(resistance);
    SerialUSB.println("*********************************");
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
  // Calculate the resistance of the NTC thermistor
  float resistance = 10000 * ((float)adcValue / (4096 - adcValue));

  // Calculate the temperature in Kelvin using the Beta parameter equation
  float temperatureK = 1 / (log(resistance / 10000) / 3950 + 1 / 298.15); // 298.15K is 25°C

  // Convert the temperature from Kelvin to Celsius
  float temperatureC = temperatureK - 273.15;

  // Round the temperature to the nearest 0.5°C
  float roundedTemperatureC = round(temperatureC * 2) / 2.0;

  // Convert the temperature to an int16_t
  int16_t temperatureInt = (int16_t)(roundedTemperatureC * 10); // Multiply by 10 to preserve the .5 precision

  return temperatureInt;
}