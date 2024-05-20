#include "driver/rmt_rx.h"

typedef struct {
  uint16_t address;
  uint16_t command;
} NECFarm;
bool isStartSymbol(rmt_symbol_word_t symbol);
bool isStopSymbol(rmt_symbol_word_t symbol);
bool isRepeat(rmt_symbol_word_t symbol0,rmt_symbol_word_t symbol1);

NECFarm necDecode(rmt_symbol_word_t* symbol);