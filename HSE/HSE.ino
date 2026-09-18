#include <Arduino.h>
#include "ADCTouchSensor.h"

void inThangCot(float v1, float v2, float v3, float v4);

// Lưu ý: nếu phần cứng tách riêng được chân xả thì nên đổi PA1 ở kênh 2 thành chân khác (ví dụ PA2)
ADCTouchSensor pressure1(PA0, PA1, 50);
ADCTouchSensor pressure2(PA3, PA1, 50);
ADCTouchSensor pressure3(PA6, PA7, 50);
ADCTouchSensor pressure4(PB1, PB0, 50);

float Ref1 = 0, Ref2 = 0;
const unsigned int SAMPLES = 90; 

void setup() {
  Serial.begin(115200);
  delay(1500);

  // Cập nhật và in thông số Clock hệ thống
  SystemCoreClockUpdate();

  Serial.println("--- THONG SO XUNG NHIP HE THONG ---");
  Serial.print("SYSCLK : ");
  Serial.print(SystemCoreClock / 1000000.0, 2);
  Serial.println(" MHz");

  uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;
  if (sws == RCC_CFGR_SWS_HSI) {
    Serial.println("Nguon Clock: HSI (Dao dong noi)");
  } else if (sws == RCC_CFGR_SWS_HSE) {
    Serial.println("Nguon Clock: HSE (Thach anh ngoai)");
  } else if (sws == RCC_CFGR_SWS_PLL) {
    if (RCC->CFGR & RCC_CFGR_PLLSRC) {
      Serial.println("Nguon Clock: PLL qua HSE (Thach anh ngoai)");
    } else {
      Serial.println("Nguon Clock: PLL qua HSI (Dao dong noi)");
    }
  }

  uint32_t pclk2 = SystemCoreClock;
  uint32_t adc_div = (RCC->CFGR & RCC_CFGR_ADCPRE);
  float adc_clock = 0;
  if (adc_div == RCC_CFGR_ADCPRE_DIV2) adc_clock = (pclk2 / 2.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV4) adc_clock = (pclk2 / 4.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV6) adc_clock = (pclk2 / 6.0) / 1000000.0;
  else if (adc_div == RCC_CFGR_ADCPRE_DIV8) adc_clock = (pclk2 / 8.0) / 1000000.0;

  Serial.print("ADC Clock: ");
  Serial.print(adc_clock, 2);
  Serial.println(" MHz");
  Serial.println("------------------------------------");

  // Thiết lập ADC 12-bit và lấy mẫu mốc ban đầu (Baseline)
  analogReadResolution(12);
  delay(100);
  Ref1 = pressure1.readRaw(300);
  Ref2 = pressure2.readRaw(300);
}

void loop() {
  uint32_t T1 = micros();
  float raw1 = pressure1.readRaw(SAMPLES);
  uint32_t T2 = micros();
  float raw2 = pressure2.readRaw(SAMPLES);
  uint32_t T3 = micros();

  uint32_t T_1 = T2 - T1;
  uint32_t T_2 = T3 - T2;
  float value1 = raw1 - Ref1;
  float value2 = raw2 - Ref2;
  Serial.print(value1);
  Serial.print("    ");
  Serial.print(value2);
  Serial.print("    ");
  Serial.print(T_1);
  Serial.print("    ");
  Serial.println(T_2);
  delay(1000);
}