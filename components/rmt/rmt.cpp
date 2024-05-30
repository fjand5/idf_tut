#include <stdio.h>
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "rmt.h"
#include "esp_log.h"
#include "nec_decode.h"
#include "algorithm"
#define WS2812B_FREQ 20000000
#define WS2812B_NS_TICK(ns) (ns / 50)
rmt_transmit_config_t transmitConfig{0, {0, 0}};
struct Color
{
  uint8_t g;
  uint8_t r;
  uint8_t b;
};
enum class Phase
{
  reset,
  data
};

Phase phase = Phase::reset;
Color color[8];
rmt_encoder_handle_t ws2812BEncoder;

void createWS2812BEncoder(rmt_encoder_handle_t *encoder)
{
  static rmt_encoder_t baseEncoder;
  static rmt_encoder_handle_t bytesEncoder = nullptr;
  static rmt_encoder_handle_t resetEncoder = nullptr;
  static rmt_symbol_word_t resetSymbol;

  resetSymbol.level0 = 0;
  resetSymbol.duration0 = WS2812B_NS_TICK(25000);

  resetSymbol.level1 = 0;
  resetSymbol.duration1 = WS2812B_NS_TICK(25000);

  rmt_new_bytes_encoder(
      new rmt_bytes_encoder_config_t{
          {
              WS2812B_NS_TICK(400),
              1,
              WS2812B_NS_TICK(850),
              0,
          },
          {
              WS2812B_NS_TICK(800),
              1,
              WS2812B_NS_TICK(450),
              0,
          },
          {0}},
      &bytesEncoder);
  rmt_new_copy_encoder(new rmt_copy_encoder_config_t, &resetEncoder);
  baseEncoder.encode = [](rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
  {
    static uint16_t count = 0;
    count += bytesEncoder->encode(bytesEncoder, tx_channel, primary_data, data_size, ret_state);
    if (count >= 8 * 24)
    {
      resetEncoder->encode(resetEncoder, tx_channel, &resetSymbol, sizeof(resetSymbol), ret_state);
      count=0;
    }
    size_t ret = 0;
    return ret;
  };

  *encoder = &baseEncoder;
}
void startRMT(void)
{
  rmt_channel_handle_t txChannel = nullptr;
  ESP_ERROR_CHECK(rmt_new_tx_channel(
      new rmt_tx_channel_config_t{
          GPIO_NUM_23,
          RMT_CLK_SRC_APB,
          20000000,
          64,
          4,
          ESP_INTR_FLAG_LEVEL1,
          {0, 0, 0, 0}},
      &txChannel));

  rmt_enable(txChannel);

  for (size_t i = 0; i < 8; i++)
  {
    color[i].r = 0;
    color[i].g = 0;
    color[i].b = 0;
  }
  createWS2812BEncoder(&ws2812BEncoder);
  rmt_tx_register_event_callbacks(
      txChannel,
      new rmt_tx_event_callbacks_t{
          [](rmt_channel_handle_t tx_chan, const rmt_tx_done_event_data_t *edata, void *user_ctx)
          {
            rmt_transmit(tx_chan, ws2812BEncoder, color, sizeof(color), &transmitConfig);
            return true;
          }},
      nullptr);
  rmt_transmit(txChannel, ws2812BEncoder, color, sizeof(color), &transmitConfig);
  uint16_t step = 0;
  while (1)
  {

    for (size_t i = 0; i < 8; i++)
    {
      color[i].r = 0;
      color[i].g = 0;
      color[i].b = 0;
      if (step % 8 == i)
      {
        color[i].r = 255;
      }
    }
    step++;
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
#if 0
#define IR_FREQ 38461
#define IR_DURATION_US(d) uint16_t(d * 1000000ul / IR_FREQ)
OnReceiveData _onReceiveData;
void recevive(rmt_channel_handle_t handle)
{

  static rmt_symbol_word_t symbols[64];
  static rmt_receive_config_t rxConfig = {
      .signal_range_min_ns = 25000,
      .signal_range_max_ns = 10000000,
  };
  rmt_receive(handle, symbols, sizeof(symbols), &rxConfig);
}
bool onReceiveDone(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
  QueueHandle_t rxQueue = (QueueHandle_t)user_ctx;
  if (uxQueueSpacesAvailable(rxQueue))
  {
    rmt_rx_done_event_data_t data = *edata;
    data.received_symbols = new rmt_symbol_word_t[data.num_symbols];
    std::copy(edata->received_symbols, edata->received_symbols + edata->num_symbols, data.received_symbols);
    xQueueSend(rxQueue, &data, portMAX_DELAY);
  }
  recevive(rx_chan);
  return pdTRUE;
}
void setOnReceiveData(OnReceiveData onReceiveData)
{
  _onReceiveData = onReceiveData;
};

void startRMT(void)
{
  static QueueHandle_t rxQueue = xQueueCreate(8, sizeof(rmt_rx_done_event_data_t));
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

  recevive(handle);
  xTaskCreate(
      [](void *p)
      {
        rmt_rx_done_event_data_t rxData;
        while (1)
        {
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
            NECFrame necFrame = necDecode(rxData.received_symbols);
            if (_onReceiveData)
            {
              _onReceiveData(necFrame);
            }
            ESP_LOGI("__NEC", "%04x -- %04x", necFrame.address, necFrame.command);
          }
          delete[] rxData.received_symbols;
        }
      },
      "__rmt_rx", 2000, nullptr, 1, nullptr);
}
#endif