#include "nec_decode.h"
typedef void(*OnReceiveData)(NECFrame frame);
void startRMT(void);
void setOnReceiveData(OnReceiveData onReceiveData);
