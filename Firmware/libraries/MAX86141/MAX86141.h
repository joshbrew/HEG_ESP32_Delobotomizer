#ifndef MAX86141_H
#define MAX86141_H
//Modified by Joshua Brewster for Arduino compatibility. Developed on the ESP32, should be compatible with most Arduino boards.
//I expanded some of the features, improved documentation. More to come. Thank you Michael Lyons for the help getting it to work.

//Found the old code here with the below copyright for the MAX86141 code: https://github.com/jonasgitt/Patient24---Remote-Patient-Monitoring

/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <stdint.h>
#include <SPI.h>

//MAX86141 Registry addresses

#define WRITE_EN 0x00
#define READ_EN 0xFF

#define REG_INT_STAT_1         (0x00)      //Interrupt Status 1
#define REG_INT_STAT_2         (0x01)      //Interrupt Status 2
#define REG_INT_EN_1           (0x02)      //Interrupt Enable 1
#define REG_INT_EN_2           (0x03)      //Interrupt Enable 2
#define REG_FIFO_WR_PTR        (0x04)      //FIFO Buffer Write Pointer
#define REG_FIFO_RD_PTR        (0x05)      //FIFO Buffer Read Pointer
#define REG_OVF_COUNTER        (0x06)      //Over Flow Counter
#define REG_FIFO_DATA_COUNT    (0x07)      //FIFO Data Counter
#define REG_FIFO_DATA          (0x08)      //FIFO Data Register
#define REG_FIFO_CONFIG_1      (0x09)      //FIFO Configuration 1
#define REG_FIFO_CONFIG_2      (0x0A)      //FIFO Configuration 2
#define REG_MODE_CONFIG        (0x0D)      //System Control

//Photoplethysmogram (PPG) registers
#define REG_PPG_SYNC_CTRL      (0x10)      //PPG Sync Control
#define REG_PPG_CONFIG_1       (0x11)      //PPG Configuration Settings Group 1
#define REG_PPG_CONFIG_2       (0x12)      //PPG Configuration Settings Group 2
#define REG_PPG_CONFIG_3       (0x13)      //PPG Configuration Settings Group 3

#define REG_PROX_INTR_THRESH   (0x14)      //Prox Interrupt Threshold
#define REG_PD_BIAS            (0x15)      //Photo Diode Bias
#define REG_PICKET_FENCE       (0x16)      //Picket Fence Settings

#define REG_LED_SEQ_1          (0x20)      //LED Sequence 1
#define REG_LED_SEQ_2          (0x21)      //LED Sequence 2
#define REG_LED_SEQ_3          (0x22)      //LED Sequence 3

#define REG_LED1_PA            (0x23)      //LED 1 Pulse Amplitude
#define REG_LED2_PA            (0x24)      //LED 2 Pulse Amplitude
#define REG_LED3_PA            (0x25)      //LED 3 Pulse Amplitude
#define REG_LED4_PA            (0x26)      //LED 4 Pulse Amplitude
#define REG_LED5_PA            (0x27)      //LED 5 Pulse Amplitude
#define REG_LED6_PA            (0x28)      //LED 6 Pulse Amplitude
#define REG_LED_PILOT_PA       (0x29)      //LED Pilot Pulse Amplitude
#define REG_LED_RANGE_1        (0x2A)      //LED Amplitude Range 1
#define REG_LED_RANGE_2        (0x2B)      //LED Amplitude Range 2

//Hi resolution DAC settings for each LED.
#define REG_S1_HI_RES_DAC1     (0x2C)
#define REG_S2_HI_RES_DAC1     (0x2D)
#define REG_S3_HI_RES_DAC1     (0x2E)
#define REG_S4_HI_RES_DAC1     (0x2F)
#define REG_S5_HI_RES_DAC1     (0x30)
#define REG_S6_HI_RES_DAC1     (0x31)

#define REG_S1_HI_RES_DAC2     (0x32)
#define REG_S2_HI_RES_DAC2     (0x33)
#define REG_S3_HI_RES_DAC2     (0x34)
#define REG_S4_HI_RES_DAC2     (0x35)
#define REG_S5_HI_RES_DAC2     (0x36)
#define REG_S6_HI_RES_DAC2     (0x37)

//Die-temp registers
#define REG_TEMP_CONFIG        (0x40)
#define REG_TEMP_INTR          (0x41)
#define REG_TEMP_FRAC          (0x42)

//SHA256 registers
#define REG_SHA_CMD            (0xF0)
#define REG_SHA_CONFIG         (0xF1)

//Memory registers
#define REG_MEM_CTRL           (0xF2)
#define REG_MEM_IDX            (0xF3)
#define REG_MEM_DATA           (0xF4)
#define REG_PART_ID            (0xFF)

/*
SPI notes:
Each byte register is 8 bits wide
EX. vspi->transfer(0x01,0b01010101) sends the binary data to the register. Hex bytes are binary shorthand
and the register applies the binary in reverse order where the index is 0b[7,6,5,4,3,2,1,0]
The datasheet describes all of the parameters you can enter.

Note: If a sample rate is set that can not be supported by the selected pulse width and number of exposures per sample, then
the highest available sample rate will be automatically set. The user can read back this register to confirm the sample rate.
ADC Output is 18 bits. Number of exposures controlled in LED sequence registers (1 to 6)

PPG_SR (sampling rate).
Hex         SPS     Pulses per Sample
0x0A        8       1
0x0B        16      1
0x0C        32      1
0x0D        64      1
0x0E        128     1
0x0F        256     1
0x10        512     1
0x11        1024    1
0x12        2048    1
0x13        4096    1

0x00        25      1
0x01        50      1
0x02        84      1
0x03        100     1
0x04        200     1
0x05        400     1
0x06        25      2
0x07        50      2
0x08        84      2
0x09        100     2

Sample Average SMP_AVE
Hex     Sample Average
0x0     1
0x1     2
0x2     4    
0x3     8
0x4     16
0x5     32
0x6     64
0x7     128

ADC Full Scale Input Current ADC_RGE
Hex     Amps
0x0     4.0uA
0x1     8.0uA
0x2     16.0uA
0x3     32.0uA

Integration time PPG_TINT
Hex     Time
0x0     14.8uS
0x1     29.4uS
0x2     58.7uS
0x3     117.3uS

LED settings (4 bit address options for REG_LED_SEQ 1, 2, and 3)
Data outputted in order of setting, then anything set to 0x00 won't be outputted to free up the FIFO for more data.
Bin     Setting
0000    NONE 
0001    LED1
0010    LED2
0011    LED3
0100    LED1 & LED2
0101    LED1 & LED3
0110    LED2 & LED3
0111    LED1, LED2, LED3
1000    Pilot on LED1
1001    DIRECT AMBIENT (i.e. normal photodiode measurements)
1010    LED4 (external mux)
1011    LED5 (external mux)
1100    LED6 (external mux)

LED Amplitude Range LEDx_RGE (LEDx_PA = 0xFF) - not sure how to set this.
Hex     Amps
0x0     31mA
0x1     62mA
0x2     93mA
0x3     124mA

LED Settling LED_SETLNG
bits    Time    
00      4uS
01      6uS (default)
10      8uS
11      12uS

Photo Diode Bias REG_PDIODE_BIAS
Hex         Photo Diode Capacitance
0x001       0pF to 65pF
0x101       65pF to 130pF
0x110       130pF to 260pF
0x111       260pF to 520pF

Select digital filter type DIG_FILT_SEL 
0x0     Use CDM
0x1     Use FDM
*/

//pseudocode from datasheet
/*
DEVICE OPEN
START;
 // AFE Initialization
WRITE RESET[0] to 0x1; // Soft Reset (Register 0x0D[0])
DELAY 1ms;
WRITE SHDN[0] to 0x1; // Shutdown (Register 0x0D[1])
READ Interrupt_Status_1; // Clear Interrupt (Register 0x00)
READ Interrupt_Status_2; // Clear Interrupt (Register 0x01)
WRITE PPG_TINT[1:0] to 0x3; // Pulse Width = 123.8ms (Register 0x11[1:0])
WRITE PPG1_ADC_RGE1:0] to 0x2; // ADC Range = 16μA (Register 0x11[3:2])
WRITE PPG2_ADC_RGE1:0] to 0x2; // ADC Range = 16μA (Register 0x11[3:2])
 // For MAX86141 when used in Dual Channel only
WRITE SMP_AVE[2:0] to 0x0; // Sample Averaging = 1 (Register 0x12[2:0])
WRITE PPG_SR[4:0] to 0x00; // Sample Rate = 25sps (Register 0x12[7:3])
WRITE LED_SETLNG[1:0] to 0x3; // LED Settling Time = 12ms (Register 0x13[7:6])
WRITE PD_BIAS1[2:0] to 0x01; // PD 1 Biasing for Cpd = 0~65pF (Register 0x15[2:0])
WRITE PD_BIAS2[2:0] to 0x01; // PD 1 Biasing for Cpd = 0~65pF (Register 0x15[2:0])
 // For MAX86141 when used in Dual Channel only
WRITE LED1_RGE[1:0] to 0x3; // LED Driver 1 Range = 124mA (Register 0x15[2:0]) <--- ???
WRITE LED2_RGE[1:0] to 0x3; // LED Driver 2 Range = 124mA (Register 0x15[2:0]) <--- ???
WRITE LED1_DRV[1:0] to 0x20; // LED 1 Drive Current = 15.36mA (Register 0x23[7:0])
WRITE LED2_DRV[1:0] to 0x20; // LED 2 Drive Current = 15.36mA (Register 0x24[7:0])
WRITE LP_Mode[0] to 0x1; // Low Power mode enabled
 // FIFO Configuration
WRITE FIFO_A_FULL[6:0] to 0xF; // FIFO INT triggered condition (Register 0x09[6:0])
WRITE FIFO_RO to 0x1; // FIFO Roll Over enabled (Register 0x0A[1])
WRITE A_FULL_EN to 0x1; // FIFO_A_FULL interrupt enabled (Register 0x02[7])
WRITE LEDC1[3:0] to 0x1; // LED1 exposure configured in time slot 1
WRITE LEDC2[3:0] to 0x2; // LED2 exposure configured in time slot 1
WRITE LEDC3[3:0] to 0x0;
WRITE LEDC4[3:0] to 0x0;
WRITE LEDC5[3:0] to 0x0;
WRITE LEDC6[3:0] to 0x0;
WRITE SHDN[0] to 0x0; // Start Sampling STOP;
*/


class MAX86141 {

  public:

    //THESE NEED TO BE SET UP MANUALLY
    SPIClass * spi = NULL;
    int SS;
    int spiClk = 1000000; //8MHz clock on MAX86141 Max, only 200KHz necessary.
    bool debug = false;
    
    int led1A[128];
    int led1B[128];
    int led2A[128];
    int led2B[128];
    int led3A[128];
    int led3B[128];
    //led4A thru led6B
    
    uint8_t       m_tx_buf[3];                       /**< TX buffer. */
    uint8_t       m_rx_buf[3];                       /**< RX buffer. */
    const uint8_t m_length = sizeof(m_tx_buf);       /**< Transfer length. */

    //Functions
    void init(int setSpiClk);
    void write_reg(uint8_t address, uint8_t data_in);
    uint8_t read_reg(uint8_t address);
    void fifo_intr();
    void read_fifo(uint8_t data_buffer[], uint8_t count);
    void device_data_read(void);
    void setSS(int pin);
    void setSPI(SPIClass * newspi);
    void setSpiClk(int newSpiClk);
    void setDebug(bool setdebug);
    void clearInt();
};

#endif 