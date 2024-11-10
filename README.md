# EE 249 Final Project

## Wiring

| Index | Board Pin | STM32 Pin (L) | STM32 Pin (R) | Description                                      |
| ----- | --------- | ------------- | ------------- | ------------------------------------------------ |
|     1 | GND       | GND           | GND           | Ground                                           |
|     2 | VIN       | 3V3           | 3V3           | Power, 3.3V ~ 5V                                 |
|     3 | SCL       | PA5 (SCLK)    | PB13 (SCLK)   | SPI clock input                                  |
|     4 | SDA       | PA7 (MOSI)    | PB15 (MOSI)   | SPI data input                                   |
|     5 | RES       | PA9           | PB2           | Reset, active low                                |
|     6 | DC        | PA8           | PB1           | Data / Command select, low: command; high: data  |
|     7 | CS        | PB6           | PA11          | SPI chip select, active low                      |
|     8 | BLK       | PC7           | PB12          | Backlight enable, default pull-up to 3V3         |

## Peripheral Allocation

### SPI

- Frame Format: Motorola
- Data Size: 8 bits
- First Bit: MSB First
- Baud Rate: 20 Mbps
- Clock Polarity: Low
- Clock Phase: 1st Edge
- NSS Signal: Software Controlled

### Timer

- Prescaler: 79
- Counter Mode: Up
- Counter Period: 19999
- PWM Generation
  - PWM Mode: Mode 1
  - Pulse: 1500
  - Output Compare Preload: Enable
  - Fast Mode: Disable
  - CH Polarity: High
