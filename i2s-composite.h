#pragma once
#include "driver/i2s.h"

static const i2s_port_t I2S_PORT = (i2s_port_t)I2S_NUM_0;

class I2SComposite {
  private:
  int buffer_length;
  public:

  I2SComposite(int buf_len) {
    buffer_length = buf_len;
    i2s_config_t i2s_config = {
       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
       .sample_rate = 1000000,  //not really used
       .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT,
       .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
       .communication_format = I2S_COMM_FORMAT_I2S_MSB,
       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
       .dma_buf_count = 2,
       .dma_buf_len = buffer_length  //a buffer per line
    };
    
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);    //start i2s driver

    i2s_set_pin(I2S_PORT, NULL);                           //use internal DAC

    i2s_set_sample_rates(I2S_PORT, 1000000);               //dummy sample rate, since the function fails at high values

    //this is the hack that enables the highest sampling rate possible 13.333MHz, have fun
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_NUM_V, 2, I2S_CLKM_DIV_NUM_S);
    SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(I2S_PORT), I2S_TX_BCK_DIV_NUM_V, 2, I2S_TX_BCK_DIV_NUM_S);
  }

  void write_line(unsigned short *line)
  {
    esp_err_t error = ESP_OK;
    size_t bytes_written = 0;
    size_t bytes_to_write = buffer_length * sizeof(unsigned short);
    size_t cursor = 0;
    while (error == ESP_OK && bytes_to_write > 0) {
      error = i2s_write(I2S_PORT, (unsigned char *)line + cursor, bytes_to_write, &bytes_written, portMAX_DELAY);
      bytes_to_write -= bytes_written;
      cursor += bytes_written;
    }
  }
};
