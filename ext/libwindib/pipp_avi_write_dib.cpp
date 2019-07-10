#include <oa_common.h>

#include <cstdlib>
//#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>  // exit()
#include "pipp_avi_write_dib.h"


// ------------------------------------------
// Constructor
// ------------------------------------------
c_pipp_avi_write_dib::c_pipp_avi_write_dib() :
  line_gap(0)
{
    vids_stream_header.handler.u32 = FCC_DIB;  // Override value from base class
    bitmap_info_header.compression.u32 = 0;  // No compression
}


// ------------------------------------------
// Set codec specific values
// ------------------------------------------
int32_t c_pipp_avi_write_dib::set_codec_values()
{
    // Set flag to write colour table if required
    if (this->colour == 0) {
        this->write_colour_table = 1;
    }

    // Calculate line gap
    line_gap = (bytes_per_pixel * width) % 4;
    if (line_gap != 0) {
        line_gap = 4 - line_gap;
    }

    // Calculate the frame size
    frame_size = (width * bytes_per_pixel + line_gap) * height;

    return 0;
}


// ------------------------------------------
// Write frame to AVI file
// ------------------------------------------
int32_t c_pipp_avi_write_dib::write_frame(
    uint8_t  *data,
    int32_t colour,
    uint32_t bpp,
    void *extra_data __attribute((unused)))
{
    int32_t ret;
    
    if (colour < 0 || colour > 2) {
        colour = 0;
    }

    // Indicate that a frame is about to be added
    frame_added();
        
    fwrite (&_00db_chunk_header , 1 , sizeof(_00db_chunk_header) , avi_fp);

    uint8_t *buffer;
    uint32_t line_length = width * bytes_per_pixel;
    if (line_gap == 0 && bytes_per_pixel == 3 && bpp == 1) {
        // Use supplied data directly
        buffer = data;
    } else {
        // Create version of image with line gaps in
        buffer = temp_buffer.get_buffer((width * bytes_per_pixel + line_gap)* height);

        if (bpp == 1) {
            if (bytes_per_pixel == 3) {
                // Colour version 
                for (int32_t y = 0; y < height; y++) {
                    uint8_t *src_ptr = data + (y * line_length);
                    uint8_t *dst_ptr = buffer + (y * (line_length + line_gap));
                    memcpy(dst_ptr, src_ptr, line_length);
                }
            } else {
                // Mono version
                // uint8_t *src_ptr = data + colour;
                uint8_t *src_ptr = data;
                uint8_t *dst_ptr = buffer;
                for (int32_t y = 0; y < height; y++) {
                    memcpy ( dst_ptr, src_ptr, width );
                    dst_ptr += width;
                    src_ptr += width;
                    // for (int32_t x = 0; x < width; x++) {
                    //     *dst_ptr++ = *src_ptr;
                    //     src_ptr += 3;
                    // }

                    dst_ptr += line_gap;
                }
            }
        } else {  // Bytes per sample == 2
            if (bytes_per_pixel == 3) {
                // Colour version 
                for (int32_t y = 0; y < height; y++) {
                    uint16_t *src_ptr = (uint16_t *)data + (y * line_length);
                    uint8_t *dst_ptr = buffer + (y * (line_length + line_gap));
                    for (uint32_t x = 0; x < line_length; x++) {
                        *dst_ptr++ = *src_ptr++ >> 8;
                    }
                }
            } else {
                // Mono version
                // uint16_t *src_ptr = (uint16_t *)data + colour;
                uint16_t *src_ptr = (uint16_t *)data;
                uint8_t *dst_ptr = buffer;
                for (int32_t y = 0; y < height; y++) {
                    memcpy ( dst_ptr, src_ptr, width * 2 );
                    dst_ptr += width;
                    src_ptr += width;
                    // for (int32_t x = 0; x < width; x++) {
                    //     *dst_ptr++ = *src_ptr >> 8;
                    //     src_ptr += 3;
                    // }

                    dst_ptr += line_gap;
                }
            }
        }
    }

    // Write image data to file
    last_frame_pos = ftell64(avi_fp);  // Grab position of last file

    ret = fwrite (buffer , 1 , (width * bytes_per_pixel + line_gap) * height, avi_fp);

    if (ret != (width * bytes_per_pixel + line_gap) * height) {
        fprintf(stderr, "Error: Error writing to AVI file\n");
        exit(-1);
    }

    return 0;
}
