MAX86141 LIBRARY FOR ARDUINO
REQUIRES A MAX86141 DEVICE


pseudocode:
MAX86141 pulseOx;
uint8_t[128] output;

void setup(){
  pulseOx.spi = new SPIClass(VSPI);
  pulseOx.SS = SSpin;
  pulseOx.spi->begin();
}

void loop(){
  uint8_t intStatus;
  read_reg(REG_INTR_STATUS_1, &intStatus); 

  if (intStatus& 0x80) //indicates full FIFO
  { 
        device_data_read();
 	Serial.print("LED1 P1: ");
  	Serial.println(pulseOx.led1A);
  	Serial.print("LED1 P2: ");
  	Serial.println(pulseOx.led1B);
  	Serial.print("LED2 P1: ");
  	Serial.println(pulseOx.led2A);
  	Serial.print("LED2 P2: ");
  	Serial.println(pulseOx.led2B);	  
  }
}

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
Register    SPS     Pulses per Sample
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
Bits    Sample Average
000     1
001     2
010     4    
011     8
100     16
101     32
110     64
111     128

ADC Full Scale Input Current ADC_RGE
Reg     Amps
0x0     4.0uA
0x1     8.0uA
0x2     16.0uA
0x3     32.0uA

Integration time PPG_TINT
Reg     Time
0x0     14.8uS
0x1     29.4uS
0x2     58.7uS
0x3     117.3uS

LED Amplitude Range LEDx_RGE (LEDx_PA = 0xFF) - not sure how to set this.
Reg     Amps
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
Register    Photo Diode Capacitance
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