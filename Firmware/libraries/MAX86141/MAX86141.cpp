#include "MAX86141.h"

/*write to register function*/
void MAX86141::write_reg(uint8_t address, uint8_t data_in)
{

/**< A buffer with data to transfer. */
    m_tx_buf[0] = address;  //Target Register
    m_tx_buf[1] = WRITE_EN; //Set Write mode
    m_tx_buf[2] = data_in;  //Byte to Write

    if(debug == true) {
        Serial.println("TX Buffer contents");
        Serial.print(m_tx_buf[0]);
        Serial.print("|");
        Serial.print(m_tx_buf[1]);
        Serial.print("|");
        Serial.println(m_tx_buf[2]);
    }
/**< A buffer for incoming data. */    

    m_rx_buf[0] = 0;
    m_rx_buf[1] = 0;
    m_rx_buf[2] = 0;

    digitalWrite(SS, HIGH);
    spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    spi->transfer(m_tx_buf, 3);
    digitalWrite(SS, HIGH);
    spi->endTransaction();
    digitalWrite(SS, LOW);
    
    m_rx_buf[0] = m_tx_buf[0];
    m_rx_buf[1] = m_tx_buf[1];
    m_rx_buf[2] = m_tx_buf[2];

    if(debug == true) {
        Serial.println("RX Buffer contents");
        Serial.print(m_rx_buf[0]);
        Serial.print("|");
        Serial.print(m_rx_buf[1]);
        Serial.print("|");
        Serial.println(m_rx_buf[2]);
    }
}

/*read register function*/
uint8_t MAX86141::read_reg(uint8_t address)
{

    /**< A buffer with data to transfer. */
    m_tx_buf[0] = address;  //Target Address
    m_tx_buf[1] = READ_EN;  //Set Read mode
    m_tx_buf[2] = 0x00;     //

/**< A buffer for incoming data. */ 
    m_rx_buf[0] = 0;
    m_rx_buf[1] = 0;
    m_rx_buf[2] = 0;

    digitalWrite(SS, HIGH);
    spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    spi->transfer(m_tx_buf,3);
    digitalWrite(SS, HIGH);
    spi->endTransaction();
    digitalWrite(SS, LOW);

    m_rx_buf[0] = m_tx_buf[0];
    m_rx_buf[1] = m_tx_buf[1];
    m_rx_buf[2] = m_tx_buf[2];

    if(debug == true){
        Serial.println("RX Buffer contents");
        Serial.print(m_rx_buf[0]);
        Serial.print("|");
        Serial.print(m_rx_buf[1]);
        Serial.print("|");
        Serial.println(m_rx_buf[2]);
    }

    return m_rx_buf[2];
}


/*inspired by pseudo-code available on MAX86141 datasheet for initialisation*/
void MAX86141::init(int newSpiClk=1000000)
{
    setSpiClk(newSpiClk);
    uint8_t temp;

	//
	// Reset part and place into shutdown mode for config.
	//
	write_reg(REG_MODE_CONFIG, 0x01);
	delay(10);
	write_reg(REG_MODE_CONFIG, 0x02);

	//
	// Clear interrupts.
	//
	read_reg(REG_INT_STAT_1);
	read_reg(REG_INT_STAT_2);

	//
	// PPG1 & 2 & 3
	//
	write_reg(REG_PPG_SYNC_CTRL, 0b000000000);
	write_reg(REG_PPG_CONFIG_1, 0b00000010); //ALC_DIS,ADD_OFF,PPG2_RGE,PPG1_RGE,PPG_TINT
	write_reg(REG_PPG_CONFIG_2, 0b10011100); //SPS (0-5), SMP_AVE (6-8)
	write_reg(REG_PPG_CONFIG_3, 0b11000110); //LED_SETLNG, DIG_FILT_SEL, BURST_EN

	//
	// LED Range = 124mA
	// LED PA LSB = 0.48mA
	// 20mA drive current setting = 20/0.48 = 42
	// LED1 = IR, 300mA max
	// LED2 = RED, 70mA max
	//
	write_reg(REG_PD_BIAS, 0b00000000);
	write_reg(REG_LED_RANGE_1, 0b00111111); // xx,LED3,LED2,LED1. 00,01,10,11 low to high
	write_reg(REG_LED1_PA, 255); // 0 = 0mA, 255 = Max mA
	write_reg(REG_LED2_PA, 255);
    write_reg(REG_LED3_PA, 255);
    //write_reg(REG_LED3_PA, 255);

	//
	// Configure FIFO.
	// LEDC1 = LED1, IR,
	// LEDC2 = LED2, RED, LEFT LED
	// LEDC3 = DIRECT AMBIENT
	// 6 samples total.
	//
	write_reg(REG_FIFO_CONFIG_1, 128 - 6);
	write_reg(REG_FIFO_CONFIG_2, 0b00001101);
	//write_reg(REG_LED_SEQ_1, 0x21);
	//write_reg(REG_LED_SEQ_2, 0x05);
	write_reg(REG_LED_SEQ_1, 0b00100011); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
	write_reg(REG_LED_SEQ_2, 0b00001001); //DATA BUF 4 | DATA BUF 3  //
    write_reg(REG_LED_SEQ_3, 0x00); // 5 | 6


	//
	// Configure interrupt.
	//
	write_reg(REG_INT_EN_1, 0x00);

	//
	// exit shutdown mode.
	//
	read_reg(REG_MODE_CONFIG);
	temp &= ~0x02;
	write_reg(REG_MODE_CONFIG, temp);

	//
	// Clear interrupts.
	//
	read_reg(REG_INT_STAT_1);
	read_reg(REG_INT_STAT_2);

	//
	// Return REG_SYS_CNTRL which should be 0x00.
	//
	temp = read_reg(REG_MODE_CONFIG);
    //Thank you Michael Lyons!
}

/* inspired by pseudo-code available on MAX86141 datasheet */
void MAX86141::device_data_read(void)
{
    uint8_t sample_count;
    uint8_t reg_val;
    uint8_t dataBuf[2304]; ///128 FIFO samples, 3 channels, 2 PDs, 3 bytes/channel 128*3*2*3 = 2304 byte buffer
      
    sample_count = read_reg(REG_FIFO_DATA_COUNT); //number of items available in FIFO to read 

    if(debug == true){
        Serial.println("Sample Count");
        Serial.println(sample_count);
    }

    read_fifo(dataBuf, sample_count);

    /*suitable formatting of data for 2 LEDs*/
    int i = 0;
    
    for (i = 0; i < sample_count; i++)
    {
        int off = 0;
        while(off < 18){
            switch(dataBuf[off] >> 3) {
                case 1:
                    led1A[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                case 2:
                    led2A[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                case 3:
                    led3A[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                    //case 4 5 6

                case 7:
                    led1B[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                case 8:
                    led2B[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                case 9:
                    led3B[i] = ((dataBuf[off] << 16) | (dataBuf[off+1] << 8) | dataBuf[off+2]) & 0x7FFFF;
                    off += 3;
                    break;

                    //case 10 11 12? check docs

            }
        }
    } 
    
    clearInt();
}

void MAX86141::fifo_intr()
{
    uint8_t count;
    count = read_reg(REG_FIFO_DATA_COUNT); 

    if (count == 0x80) //indicates full FIFO
    { 
        device_data_read();
    }
 }


/*read FIFO*/
void MAX86141::read_fifo(uint8_t data_buffer[], uint8_t count)
{
    data_buffer[0] = REG_FIFO_DATA;
	data_buffer[1] = READ_EN;		
	digitalWrite(SS, HIGH);
	spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
	digitalWrite(SS, LOW);
	spi->transfer(data_buffer, 2);
	spi->transfer(data_buffer, count * 3);
	digitalWrite(SS, HIGH);
	spi->endTransaction();
	digitalWrite(SS, LOW);

    if(debug == true){
        Serial.println("Data buffer");
        Serial.println(data_buffer[0]);
    }

}

void MAX86141::setSS(int pin){
    SS = pin;
}

void MAX86141::setSPI(SPIClass * newspi){
    spi = newspi;
}

void MAX86141::setSpiClk(int newSpiClk) {
    spiClk = newSpiClk;
}

void MAX86141::setDebug(bool setdebug) {
    debug = setdebug;
}

void MAX86141::clearInt() {
    uint16_t intr = 0x00;
	intr = read_reg(REG_INT_STAT_1) << 8;
	intr |= read_reg(REG_INT_STAT_2);
}