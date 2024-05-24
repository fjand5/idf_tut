#pragma once
#include "driver/rmt_rx.h"

struct NECFrame
{
  uint16_t address;
  uint16_t command;
  bool operator==(NECFrame other){
    return address == other.address && command == other.command;
  }
};
bool isStartSymbol(rmt_symbol_word_t symbol);
bool isStopSymbol(rmt_symbol_word_t symbol);
bool isRepeat(rmt_symbol_word_t symbol0, rmt_symbol_word_t symbol1);

NECFrame necDecode(rmt_symbol_word_t *symbol);