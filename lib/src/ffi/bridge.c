#include <stdint.h>
#include "ppu.h"

uint32_t* ppu_get_framebuffer() {
    return framebuffer;
}
