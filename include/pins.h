#ifndef PINS_H
#define PINS_H

// LED
#define TEST_LED PB15
#define SDB PB5
// IGNITION CIRCUIT
#define CAP_SW PC0
#define IGN_SW1 PC1
#define IGN_SW2 PC2
#define CAP_DIS PC3
// CHANNELS
#define CH1 PC4
#define CH2 PC5
#define CH3 PA9
#define CH4 PA10
#define CH5 PA15
#define CH6 PC8
#define CH7 PC9
#define CH8 PD0
#define CH9 PD1
#define CH10 PD2
#define CH11 PD3
#define CH12 PD4
#define CH13 PD5
#define CH14 PD6
#define CH15 PB3
#define CH16 PB4
// I2C
#define SENSOR_SDA PB7
#define SENSOR_SCL PB6
#define OLED_SDA PB11
#define OLED_SCL PB10
//UART
#define BATTERY_TX PA2
#define BATTERY_RX PA3
#define RS485_1_TX PB8
#define RS485_1_RX PB9
#define RS485_1_DIR PC12
#define RS485_2_TX PD8
#define RS485_2_RX PD9
#define RS485_2_DIR PC7
#define EXPANSION_TX PC10
#define EXPANSION_RX PC11
//BATTERY
#define OE_LVL PC13
#define NRST_BAT PC6
#define BOOT0_BAT PB14
//NRF24
#define NRF24_ON PC14
#define NRF24_FLG PC15
#define NRF24_CE PB12
#define NRF24_CS PA4
#define NRF24_SCK PA5
#define NRF24_MISO PA6
#define NRF24_MOSI PA7
//FRAM
#define FRAM_WP PA8
// ADDRESS
#define LED_DRIVER_ADDRESS 0x3C
#define FRAM_ADDRESS 0x50
#define OLED_ADDRESS 0x3C

uint8_t channel_pins[16] = {CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};

#endif