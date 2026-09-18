#include <Arduino.h>
#include "ADCTouchSensor.h"

extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  // 1. HSI 8MHz / 2 * 12 = 48MHz
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12; // 48MHz
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // 2. Bus APB1 = 24MHz, APB2 = 48MHz (Latency = 1)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  // 3. ADC Clock = 48MHz / 4 = 12MHz
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  SystemCoreClockUpdate();
}

ADCTouchSensor* pressure1 = nullptr;
ADCTouchSensor* pressure2 = nullptr;

float Ref1 = 0, Ref2 = 0;
const unsigned int SAMPLES = 90;

void setup() {
  // Thống nhất dùng Serial1 (PA9-TX, PA10-RX)
  Serial1.begin(115200);
  delay(1000);

  SystemCoreClockUpdate();

  Serial1.println("--- THONG SO XUNG NHIP HE THONG ---");
  Serial1.print("SYSCLK : ");
  Serial1.print(SystemCoreClock / 1000000.0, 2);
  Serial1.println(" MHz");

  uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;
  if (sws == RCC_CFGR_SWS_HSI) {
    Serial1.println("Nguon Clock: HSI (Dao dong noi)");
  } else if (sws == RCC_CFGR_SWS_HSE) {
    Serial1.println("Nguon Clock: HSE (Thach anh ngoai)");
  } else if (sws == RCC_CFGR_SWS_PLL) {
    if (RCC->CFGR & RCC_CFGR_PLLSRC) {
      Serial1.println("Nguon Clock: PLL qua HSE (Thach anh ngoai)");
    } else {
      Serial1.println("Nguon Clock: PLL qua HSI (Dao dong noi)");
    }
  }

  uint32_t pclk2 = SystemCoreClock; 
  uint32_t adc_div = (RCC->CFGR & RCC_CFGR_ADCPRE);
  float adc_clock = 0;
  if (adc_div == RCC_CFGR_ADCPRE_DIV2) adc_clock = (pclk2 / 2.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV4) adc_clock = (pclk2 / 4.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV6) adc_clock = (pclk2 / 6.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV8) adc_clock = (pclk2 / 8.0) / 1000000.0;

  Serial1.print("ADC Clock: ");
  Serial1.print(adc_clock, 2);
  Serial1.println(" MHz");
  Serial1.println("------------------------------------");

  // Khởi tạo cảm biến và baseline
  pressure1 = new ADCTouchSensor(PA0, PA1, 50);
  pressure2 = new ADCTouchSensor(PA3, PA2, 50);

  analogReadResolution(12);
  
  Ref1 = pressure1->readRaw(500);
  Ref2 = pressure2->readRaw(500);

  Serial1.println("--- Bat dau do ---");
}

void loop() {
  if (pressure1 == nullptr || pressure2 == nullptr) return;

  uint32_t T1 = micros();
  float raw1 = pressure1->readRaw(SAMPLES);
  uint32_t T2 = micros();
  float raw2 = pressure2->readRaw(SAMPLES);
  uint32_t T3 = micros();

  uint32_t T_1 = T2 - T1;   
  uint32_t T_2 = T3 - T2;
  float value1 = raw1 - Ref1;
  float value2 = raw2 - Ref2;

  Serial1.print(value1, 2);
  Serial1.print("    ");
  Serial1.print(value2, 2);
  Serial1.print("    ");
  Serial1.print(T_1 / 1000.0, 2); // In thời gian đo tích phân Kênh 1 (ms)
  Serial1.print(" ms    ");
  Serial1.print(T_2 / 1000.0, 2); // In thời gian đo tích phân Kênh 2 (ms)
  Serial1.println(" ms");

  delay(1000);
}