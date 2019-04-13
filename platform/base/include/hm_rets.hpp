#pragma once

#include <cstdint>

typedef int16_t hm_err_t;

#define HM_OK                               0;
#define HM_ERR_FAIL                         -1;

#define HM_ERR_MEM_ERR                      0x8001;
#define HM_ERR_INVALID_ARG                  0x8002;
#define HM_ERR_INVALID_STATE                0x8003;
#define HM_ERR_INVALID_SIZE                 0x8004;
#define HM_ERR_NOT_FOUND                    0x8005;
#define HM_ERR_NOT_SUPPORTED                0x8006;
#define HM_ERR_INVALID_RESPONSE             0x8008;
#define HM_ERR_INVALID_CHECKSUM             0x8009;
#define HM_ERR_INVALID_VERSION              0x800A;
#define HM_ERR_TIMEOUT                      0x800B;

