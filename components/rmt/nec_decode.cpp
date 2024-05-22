#include "nec_decode.h"
#define CHECK_IN_RANGE(val, min, max) (((val) > (min)) && ((val) < (max)))

bool isStartSymbol(rmt_symbol_word_t symbol)
{
  return CHECK_IN_RANGE(symbol.duration0, 8000, 10000) && CHECK_IN_RANGE(symbol.duration1, 3500, 5500);
};
bool isStopSymbol(rmt_symbol_word_t symbol)
{
  return CHECK_IN_RANGE(symbol.duration0, 460, 660) && (symbol.duration1 == 0);
};
bool isRepeat(rmt_symbol_word_t symbol0, rmt_symbol_word_t symbol1)
{
  return CHECK_IN_RANGE(symbol0.duration0, 8000, 10000) && CHECK_IN_RANGE(symbol0.duration1, 1250, 3250) &&
         CHECK_IN_RANGE(symbol1.duration0, 460, 660) && (symbol1.duration1 == 0);
};
bool logic(rmt_symbol_word_t symbol)
{
  return CHECK_IN_RANGE(symbol.duration0, 460, 660) && CHECK_IN_RANGE(symbol.duration0 + symbol.duration1, 2150, 2350);
};
NECFrame necDecode(rmt_symbol_word_t *symbol)
{
  NECFrame ret;
  ret.address = 0;
  ret.command = 0;

  for (size_t i = 0; i < 16; i++)
  {
    ret.address |= logic(symbol[i + 1]) << i;
  }
  for (size_t i = 0; i < 16; i++)
  {
    ret.command |= logic(symbol[i + 17]) << i;
  }
  return ret;
};
