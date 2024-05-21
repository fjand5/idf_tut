#include "nec_decode.h"
typedef void(*OnReceiveData)(NECFarm farm);
void startRMT(void);
void setOnReceiveData(OnReceiveData onReceiveData);
