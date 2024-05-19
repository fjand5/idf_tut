#include <stdio.h>
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "rmt.h"
bool onReceiveDone(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
  QueueHandle_t rxQueue = (QueueHandle_t)user_ctx;
  xQueueSend(rxQueue, edata, portMAX_DELAY);
  return pdTRUE;
}
void startRMT(void)
{
  static QueueHandle_t rxQueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
  rmt_rx_channel_config_t config;
  static rmt_channel_handle_t handle;
  config.clk_src = RMT_CLK_SRC_REF_TICK;
  config.gpio_num = GPIO_NUM_19;
  config.mem_block_symbols = 64;
  config.resolution_hz = 38000;
  config.intr_priority = ESP_INTR_FLAG_LEVEL1;
  ESP_ERROR_CHECK(rmt_new_rx_channel(&config, &handle));
  rmt_rx_event_callbacks_t ecb;
  ecb.on_recv_done = onReceiveDone;
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(handle, &ecb, rxQueue));
  ESP_ERROR_CHECK(rmt_enable(handle));

  rmt_symbol_word_t symbols[64];
  rmt_receive_config_t rxConfig;
  rxConfig.signal_range_max_ns = 10000000;
  rxConfig.signal_range_min_ns = 25000;

  rmt_rx_done_event_data_t rxData;
  while (1)
  {
    rmt_receive(handle, symbols, sizeof(symbols), &rxConfig);
    xQueueReceive(rxQueue, &rxData, portMAX_DELAY);
    if(rxData.num_symbols != 34 && rxData.num_symbols != 2) continue;
    for (size_t i = 0; i < rxData.num_symbols; i++)
    {
      rmt_symbol_word_t symbol = rxData.received_symbols[i];
      printf("%d -- %d \r\n", symbol.duration0 * 26, symbol.duration1* 26);
    }
  }
}
