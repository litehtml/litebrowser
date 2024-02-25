#include "globals.h"

cairo_surface_t* dib_to_surface(CTxDIB& img)
{
	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img.getWidth(), img.getHeight());
	unsigned char* dst = cairo_image_surface_get_data(surface);
	unsigned char* src = (unsigned char*)img.getBits();
	int line_size = img.getWidth() * 4;
	int dst_offset = img.getWidth() * (img.getHeight() - 1) * 4;
	int src_offset = 0;
	for (int i = 0; i < img.getHeight(); i++, src_offset += line_size, dst_offset -= line_size)
	{
		memcpy(dst + dst_offset, src + src_offset, line_size);
	}
	cairo_surface_mark_dirty(surface);
	return surface;
}
