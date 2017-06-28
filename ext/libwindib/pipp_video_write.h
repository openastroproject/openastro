#ifndef PIPP_VIDEO_WRITE_H
#define PIPP_VIDEO_WRITE_H

//#include <cstdint>


class c_pipp_video_write {
    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
        // ------------------------------------------
        // virtual destructor
        // ------------------------------------------
        virtual ~c_pipp_video_write()
        {
        };


        // ------------------------------------------
        // Return the open state of the video file
        // ------------------------------------------
        virtual int32_t get_open() = 0;


        // ------------------------------------------
        // Create a new video file
        // ------------------------------------------
        virtual int32_t create(
            const char *filename,
            int32_t  width,
            int32_t  height,
            int32_t  colour,
            int32_t  fps_rate,
            int32_t  fps_scale,
            int32_t  old_avi_format,
            int32_t  quality,
            void *extra_data = NULL) = 0;
            
            
        // ------------------------------------------
        // Write frame to AVI file
        // ------------------------------------------
        virtual int32_t write_frame(
            uint8_t *data,
            int32_t colour,
            uint32_t bpp,
            void *extra_data = NULL) = 0;
        
        // ------------------------------------------
        // Write header and close AVI file
        // ------------------------------------------
        virtual int32_t close() = 0;
};

    
#endif  // PIPP_VIDEO_WRITE_H
