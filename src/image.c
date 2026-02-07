#include "image.h"

void image8_init(Image8_t *self, uint32_t width, uint32_t height)
{
    self->width = width;
    self->height = height;
    self->data = 0;
}
