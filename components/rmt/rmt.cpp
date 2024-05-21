#include <stdio.h>
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "rmt.h"
#include "esp_log.h"
#include "nec_decode.h"

#define IR_FREQ 38461
#define IR_DURATION_US(d) uint16_t(d * 1000000ul / IR_FREQ)
OnReceiveData _onReceiveData;
bool onReceiveDone(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
  QueueHandle_t rxQueue = (QueueHandle_t)user_ctx;
  xQueueSend(rxQueue, edata, portMAX_DELAY);
  return pdTRUE;
}
void setOnReceiveData(OnReceiveData onReceiveData)
{
  _onReceiveData = onReceiveData;
};

void startRMT(void)
{
  static QueueHandle_t rxQueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
  rmt_rx_channel_config_t config = {};
  static rmt_channel_handle_t handle;
  config.clk_src = RMT_CLK_SRC_REF_TICK;
  config.gpio_num = GPIO_NUM_19;
  config.mem_block_symbols = 64;
  config.resolution_hz = IR_FREQ;
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
    if (rxData.num_symbols != 34 && rxData.num_symbols != 2)
      continue;
    for (size_t i = 0; i < rxData.num_symbols; i++)
    {
      rxData.received_symbols[i].duration0 = IR_DURATION_US(rxData.received_symbols[i].duration0);
      rxData.received_symbols[i].duration1 = IR_DURATION_US(rxData.received_symbols[i].duration1);
    }
    if (rxData.num_symbols == 2 &&
        isRepeat(rxData.received_symbols[0], rxData.received_symbols[1]))
    {
      ESP_LOGI("__NEC", "repeat");
    }
    if (rxData.num_symbols == 34 &&
        isStartSymbol(rxData.received_symbols[0]) &&
        isStopSymbol(rxData.received_symbols[33]))
    {
      NECFarm necFarm = necDecode(rxData.received_symbols);
      if (_onReceiveData)
      {
        _onReceiveData(necFarm);
      }
      ESP_LOGI("__NEC", "%04x -- %04x", necFarm.address, necFarm.command);
    }
  }
}
