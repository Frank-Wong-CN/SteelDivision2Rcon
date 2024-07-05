#include "macro.h"

#undef MAGIC
#undef BRACP0
#undef MAGIC1
#undef MAGIC2
#undef MAGIC3
#undef MAGIC4
#define MAGIC(...) PASTE_E(MAGIC_U(BRAC0_A PARN(__VA_ARGS__)))
#define BRACP0(X) SECOND(CALLDETECT X MAGIC1)X
#define MAGIC1(...) {PASTE_E(MAGIC1_U(BRAC1_A PARN(__VA_ARGS__)))}
#define MAGIC2(...) QJsonObject {{PASTE_E(MAGIC2_U(BRAC2_A PARN(__VA_ARGS__)))}}
#define MAGIC3(...) QJsonObject {{PASTE_E(MAGIC3_U(BRAC3_A PARN(__VA_ARGS__)))}}
#define MAGIC4(...) QJsonObject {{PASTE_E(MAGIC4_U(BRAC4_A PARN(__VA_ARGS__)))}}
