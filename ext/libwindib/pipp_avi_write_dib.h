#ifndef PIPP_AVI_WRITE_DIB_H
#define PIPP_AVI_WRITE_DIB_H

#include "pipp_video_write.h"
#include "pipp_avi_write.h"


class c_pipp_avi_write_dib: public c_pipp_avi_write {
private:
    int32_t line_gap;


public:
    // ------------------------------------------
    // Constructor
    // ------------------------------------------
    c_pipp_avi_write_dib();


    // ------------------------------------------
    // Write frame to AVI file
    // ------------------------------------------
    virtual int32_t write_frame(
        uint8_t *data,
        int32_t colour,
        uint32_t bpp,
        void *extra_data = NULL);


private:
    // ------------------------------------------
    // Set codec specific values
    // ------------------------------------------
    virtual int32_t set_codec_values();
};



#endif  // PIPP_AVI_WRITE_DIB_H