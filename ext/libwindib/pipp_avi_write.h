#ifndef PIPP_ODML_WRITE_H
#define PIPP_ODML_WRITE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

//#include <cstdint>
#include "pipp_video_write.h"
#include "pipp_buffer.h"

#define DEBUGF //printf

// 64-bit fseek for various platforms
#if defined(__linux__) || defined(HAVE_FSEEKO64)
#define fseek64 fseeko64
#else
#ifdef HAVE_FSEEKI64
#define fseek64 _fseeki64  // Windows
#else
#define fseek64 fseek
#endif
#endif

#if defined(__linux__) || defined(HAVE_FTELLO64)
#define ftell64 ftello64
#else
#ifdef HAVE_FTELLI64
#define ftell64 _ftelli64  // Windows
#else
#define ftell64 ftell
#endif
#endif

// AVI Flags
#define AVIF_HASINDEX 0x00000010
#define AVIF_MUSTUSEINDEX 0x00000020
#define AVIF_ISINTERLEAVED 0x00000100
#define AVIF_TRUSTCKTYPE 0x00000800
#define AVIF_WASCAPTUREFILE 0x00010000
#define AVIF_COPYRIGHTED 0x00020000

// AVI Index Flags
#define AVIIF_LIST      0x00000001  // Points to LIST not a CHUNK
#define AVIIF_KEYFRAME  0x00000010  // This frame is a key frame
#define AVIIF_FIRSTPART 0x00000020
#define AVIIF_LASTPART  0x00000040
#define AVIIF_NOTIME    0x00000100  // This frame takes no time 

// FCC codes
#define FCC_RIFF 0x46464952
#define FCC_AVI  0x20495641
#define FCC_AVIX 0x58495641
#define FCC_LIST 0x5453494C
#define FCC_JUNK 0x4b4e554a
#define FCC_AVIX 0x58495641
#define FCC_hdrl 0x6C726468
#define FCC_avih 0x68697661
#define FCC_vids 0x73646976
#define FCC_7Fxx 0x78784637
#define FCC_idx1 0x31786469
#define FCC_movi 0x69766f6d
#define FCC_00db 0x62643030
#define FCC_odml 0x6C6D646F
#define FCC_indx 0x78646E69
#define FCC_strl 0x6C727473
#define FCC_strh 0x68727473
#define FCC_strf 0x66727473
#define FCC_strn 0x6E727473
#define FCC_dmlh 0x686C6D64
#define FCC_ix00 0x30307869

#define FCC_BI_RGB 0x00000000
#define FCC_RGB    0x20424752
#define FCC_Y800   0x30303859
#define FCC_DIB    0x20424944
#define FCC_UYVY   0x59565955
#define FCC_YUY2   0x32595559
#define FCC_YUYV   0x56595559
#define FCC_I420   0x30323449
#define FCC_IYUV   0x56555949
#define FCC_YV12   0x32315659
#define FCC_BY8    0x20385942

#define NUMBER_SUPERINDEX_ENTRIES 62


class c_pipp_avi_write: public c_pipp_video_write {
    // ------------------------------------------
    // Protected definitions
    // ------------------------------------------
    protected:
        // Member structures
        struct s_list_header {
            union {
                char chr[4];
                uint32_t u32;
            } list;
            uint32_t size;
            union {
                char chr[4];
                uint32_t u32;
            } four_cc;
        };

        struct s_chunk_header {
            union {
                char chr[4];
                uint32_t u32;
            } four_cc;
            uint32_t size;
        };

        struct s_main_avi_header {
            uint32_t micro_sec_per_frame; // frame display rate (or 0)
            uint32_t max_bytes_per_sec; // max. transfer rate
            uint32_t padding_granularity; // pad to multiples of this size;
            uint32_t flags; // the ever-present flags
            uint32_t total_frames; // # frames in file
            uint32_t initial_frames;
            uint32_t streams;
            uint32_t suggested_buffer_size;
            int32_t width;
            int32_t height;
            uint32_t reserved[4];
        };

        struct s_avi_stream_header {
            union {
                char chr[4];
                uint32_t u32;
            } type;
            union {
                char chr[4];
                uint32_t u32;
            } handler;
            uint32_t flags;
            uint16_t priority;
            uint16_t language;
            uint32_t initial_frames;
            uint32_t scale;
            uint32_t rate; /* dwRate / dwScale == samples/second */
            uint32_t start;
            uint32_t length; /* In units above... */
            uint32_t suggested_buffer_size;
            uint32_t quality;
            uint32_t sample_size;
            struct {
                short int left;
                short int top;
                short int right;
                short int bottom;
            }  frame;
        };

        struct s_bitmap_info_header {
            uint32_t size;
            int32_t width;
            int32_t height;
            uint16_t planes;
            uint16_t bit_count;
            union {
                char chr[4];
                uint32_t u32;
            } compression;
            uint32_t size_image;
            uint32_t x_pels_per_meter;
            uint32_t y_pels_per_meter;
            uint32_t clr_used;
            uint32_t clr_important;
        };

        struct s_avi_old_index_entry {
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            uint32_t flags;
            uint32_t offset;
            uint32_t size;
        };

        struct s_avi_superindex_header {
            int16_t longs_per_entry;
            int8_t index_sub_type;
            int8_t index_type;
            int32_t entries_in_use;
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            uint32_t reserved[3];
        };

        struct s_avi_superindex_entry {
            int64_t offset;
            int32_t size;
            int32_t duration;
        };

        struct s_extended_avi_header {
            int32_t total_frames;
            int32_t reserved[61];  // Hardly documented!
        };

        struct s_avi_stdindex_header {
            int16_t longs_per_entry;
            int8_t index_sub_type;
            int8_t index_type;
            int32_t entries_in_use;
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            int32_t base_offset[2];
            int32_t reserved3;
        };

        struct s_avi_stdindex_entry {
            int32_t offset;
            int32_t size;
        };

        // Member variables
        char *p_filename;
        char *p_extension;
        FILE *avi_fp;
        int32_t open;
        int32_t split_count;
        int32_t old_avi_format;
        int32_t width;
        int32_t height;
        int32_t frame_size;
        int32_t colour;
        uint32_t max_frames_in_first_riff;
        uint32_t max_frames_in_other_riffs;
        int32_t write_colour_table;
        int32_t total_frame_count;
        uint32_t current_frame_count;
        int32_t riff_count;
        int32_t bytes_per_pixel;
        int64_t last_frame_pos;
        int64_t riff_start_position;

        c_pipp_buffer temp_buffer;

        // Various list and chunk structures in main file
        s_list_header avi_riff_header;
        s_list_header avix_riff_header;
        s_list_header hdrl_list_header;
        s_chunk_header avih_chunk_header;
        s_main_avi_header main_avih_header;
        s_list_header strl_list_header;
        s_chunk_header strh_chunk_header;
        s_avi_stream_header vids_stream_header;
        s_chunk_header strf_chunk_header;
        s_bitmap_info_header bitmap_info_header;
        s_chunk_header junk_chunk_header;

        // New odml stuff start
        s_chunk_header indx_chunk_header;
        s_avi_superindex_header avi_superindex_header;
        s_avi_superindex_entry avi_superindex_entries[NUMBER_SUPERINDEX_ENTRIES];
        s_list_header odml_list_header;
        s_chunk_header dmlh_chunk_header;
        s_extended_avi_header extended_avi_header;
        // New odml stuff end

        s_list_header movi_list_header;
        s_list_header movi_avix_list_header;
        s_chunk_header _00db_chunk_header;
        s_chunk_header idx1_chunk_header;
        s_avi_old_index_entry avi_index_entry;

        s_chunk_header ix00_chunk_header;
        s_avi_stdindex_header avi_stdindex_header;
        s_avi_stdindex_entry avi_stdindex_entry;


    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
    
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_pipp_avi_write();


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        virtual ~c_pipp_avi_write() {
        };


        // ------------------------------------------
        // Return the open state of the AVI file
        // ------------------------------------------
        int32_t get_open() {
            return open;
        };


        // ------------------------------------------
        // Create a new AVI file
        // ------------------------------------------
        int32_t create(
            const char *filename,
            int32_t  width,
            int32_t  height,
            int32_t  colour,
            int32_t  fps_rate,
            int32_t  fps_scale,
            int32_t  old_avi_format,
            int32_t  quality,
            void *extra_data = NULL);
            
            
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
        int32_t close();

    protected:
        // ------------------------------------------
        // Write headers to file
        // ------------------------------------------
        int32_t write_headers();


        // ------------------------------------------
        // A new frame has been added
        // ------------------------------------------
        void frame_added();


        // ------------------------------------------
        // Write debug output
        // ------------------------------------------
        int32_t debug_headers();


        // ------------------------------------------
        // Set codec specific values
        // ------------------------------------------
        virtual int32_t set_codec_values() = 0;

    private:
        // ------------------------------------------
        // Finish the current RIFF
        // ------------------------------------------
        void finish_riff();
        int32_t split_create();

        // ------------------------------------------
        // Write header and close AVI file
        // ------------------------------------------
        int32_t split_close();
};

    
#endif  // PIPP_ODML_WRITE_H
