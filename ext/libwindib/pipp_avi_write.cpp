#include <oa_common.h>

#include <cstdlib>
//#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include "pipp_avi_write.h"
#include "pipp_utf8.h"

#include <cwchar>


using namespace std;


//
// index_type codes
//
#define AVI_INDEX_OF_INDEXES   0x00  // when each entry in aIndex array points to an index chunk
#define AVI_INDEX_OF_CHUNKS    0x01  // when each entry in aIndex array points to a chunk in the file
#define AVI_INDEX_IS_DATA      0x80  // when each entry is aIndex is really the data

//
// index_sub_type codes for INDEX_OF_CHUNKS
//
#define AVI_INDEX_2FIELD 0x01 // when fields within frames are also indexed


// ------------------------------------------
// Constructor
// ------------------------------------------
c_pipp_avi_write::c_pipp_avi_write() :
    p_filename(NULL),
    p_extension(NULL),
    avi_fp(NULL),
    open(0),
    split_count(0),
    old_avi_format(0),
    width(0),
    height(0),
    frame_size(0),
    colour(0),
    write_colour_table(0),
    total_frame_count(0),
    current_frame_count(0),
    riff_count(0),
    bytes_per_pixel(1),
    last_frame_pos(0)
{
    // Initialise RIFF AVI header
    avi_riff_header.list.u32 = FCC_RIFF;
    avi_riff_header.size = 0;
    avi_riff_header.four_cc.u32 = FCC_AVI;

    // Initialise the RIFF AVIX header
    avix_riff_header.list.u32 = FCC_RIFF;
    avix_riff_header.size = 0;
    avix_riff_header.four_cc.u32 = FCC_AVIX;

    // Initialise hdrl list header
    hdrl_list_header.list.u32 = FCC_LIST;
    hdrl_list_header.size = 0;
    hdrl_list_header.four_cc.u32 = FCC_hdrl;

    // Initialise avih chunk header
    avih_chunk_header.four_cc.u32 = FCC_avih;
    avih_chunk_header.size = sizeof(s_main_avi_header);

    // Initialise the junk chunk header
    junk_chunk_header.four_cc.u32 = FCC_JUNK;
    junk_chunk_header.size = 0;

    // Initialise the main AVI header
    main_avih_header.micro_sec_per_frame = 100000;  // 10 fps by default
    main_avih_header.max_bytes_per_sec = 0;
    main_avih_header.padding_granularity = 0;
    main_avih_header.flags = AVIF_HASINDEX;// | AVIF_MUSTUSEINDEX;
    main_avih_header.total_frames = 0;  // Increment as frames are added
    main_avih_header.initial_frames = 0;  // Always 0
    main_avih_header.streams = 1;  // Always 1 stream
    main_avih_header.suggested_buffer_size = 0;  // Fill in later
    main_avih_header.width = 0;  // Fill in later
    main_avih_header.height = 0;  // Fill in later
    main_avih_header.reserved[0] = 0;  // Always 0
    main_avih_header.reserved[1] = 0;  // Always 0
    main_avih_header.reserved[2] = 0;  // Always 0
    main_avih_header.reserved[3] = 0;  // Always 0

    // Initialise strl list header
    strl_list_header.list.u32 = FCC_LIST;
    strl_list_header.size = 0;
    strl_list_header.four_cc.u32 = FCC_strl;

    // Initialise strh chunk header
    strh_chunk_header.four_cc.u32 = FCC_strh;
    strh_chunk_header.size = sizeof(s_avi_stream_header);

    // Initialise vids stream header
    vids_stream_header.type.u32 = FCC_vids;
    vids_stream_header.handler.u32 = FCC_DIB;
    vids_stream_header.flags = 0;
    vids_stream_header.priority = 0;
    vids_stream_header.language = 0;
    vids_stream_header.initial_frames = 0;
    vids_stream_header.scale = 1;
    vids_stream_header.rate = 10;  // 10 fps by default
    vids_stream_header.start = 0;
    vids_stream_header.length = 0;  // Fill in later
    vids_stream_header.suggested_buffer_size = 0;  // Fill in later
    vids_stream_header.quality = 0xFFFFFFFF;
    vids_stream_header.sample_size = 0;
    vids_stream_header.frame.left = 0;
    vids_stream_header.frame.top = 0;
    vids_stream_header.frame.right = 0;
    vids_stream_header.frame.bottom = 0;

    // Initialise STRF chunk header
    strf_chunk_header.four_cc.u32 = FCC_strf;
    strf_chunk_header.size = 0;  // Fill in later

    // Initialise the BITMAP info header
    bitmap_info_header.size = 0;  // Fill in later
    bitmap_info_header.width = 0;  // Fill in later
    bitmap_info_header.height = 0;  // Fill in later
    bitmap_info_header.planes = 1;
    bitmap_info_header.bit_count = 8;  // Fill in later
    bitmap_info_header.compression.u32 = 0;  // No compression
    bitmap_info_header.size_image = 0;  // Fill in later
    bitmap_info_header.x_pels_per_meter = 0;
    bitmap_info_header.y_pels_per_meter = 0;
    bitmap_info_header.clr_used = 0;  // Fill in later
    bitmap_info_header.clr_important = 0;

    // Initialise the indx chunk header
    indx_chunk_header.four_cc.u32 = FCC_indx;
    indx_chunk_header.size = sizeof(s_avi_superindex_header)
                           + sizeof(s_avi_superindex_entry) * NUMBER_SUPERINDEX_ENTRIES;

    // Initialise the AVI superindex header
    avi_superindex_header.longs_per_entry = 4;
    avi_superindex_header.index_sub_type = 0;//AVI_INDEX_OF_CHUNKS;
    avi_superindex_header.index_type = AVI_INDEX_OF_INDEXES;
    avi_superindex_header.entries_in_use = 0;  // Will be increment as needed
    avi_superindex_header.chunk_id.u32 = FCC_00db;
    avi_superindex_header.reserved[0] = 0;
    avi_superindex_header.reserved[1] = 0;
    avi_superindex_header.reserved[2] = 0;

    // Initialise the AVI superindex entries
    for (int x = 0; x < NUMBER_SUPERINDEX_ENTRIES; x++) {
        avi_superindex_entries[x].offset = 0;    // Fill in later
        avi_superindex_entries[x].size = 0;      // Fill in later - the size of the standard index this entry point to
        avi_superindex_entries[x].duration = 0;  // Fill in later - the number of frames in the sub-index
    }

    // Initialise the odml list header
    odml_list_header.list.u32 = FCC_LIST;
    odml_list_header.size = sizeof(odml_list_header.four_cc.u32) + sizeof(dmlh_chunk_header) + sizeof(extended_avi_header);
    odml_list_header.four_cc.u32 = FCC_odml;

    // Initialise the dmlh chunk header
    dmlh_chunk_header.four_cc.u32 = FCC_dmlh;
    dmlh_chunk_header.size = sizeof(extended_avi_header);

    // Initialise the extended AVI header
    extended_avi_header.total_frames = 0;  // Fill in as the file closes

    // Initialise the movie list header (AVI RIFF version)
    movi_list_header.list.u32 = FCC_LIST;
    movi_list_header.size = 0;  // Fill in later
    movi_list_header.four_cc.u32 = FCC_movi;

    // Initialise the movie list header (AVIX RIFF version)
    movi_avix_list_header.list.u32 = FCC_LIST;
    movi_avix_list_header.size = 0;  // Fill in later
    movi_avix_list_header.four_cc.u32 = FCC_movi;

    // Initialise 00db chunk header
    _00db_chunk_header.four_cc.u32 = FCC_00db;
    _00db_chunk_header.size = 0;  // Fill in later

    // Initialise idx1 chunk header
    idx1_chunk_header.four_cc.u32 = FCC_idx1;
    idx1_chunk_header.size = 0;  // Fill in with frames * sizeof(s_avi_old_index_entry)

    // Initialise the ix00 chunk header
    ix00_chunk_header.four_cc.u32 = FCC_ix00;
    ix00_chunk_header.size = 0;  // Fill in later

    // Initialise the standard index header
    avi_stdindex_header.longs_per_entry = 2;
    avi_stdindex_header.index_sub_type = 0;
    avi_stdindex_header.index_type = AVI_INDEX_OF_CHUNKS;
    avi_stdindex_header.entries_in_use = 0;
    avi_stdindex_header.chunk_id.u32 = FCC_00db;
    avi_stdindex_header.base_offset[0] = 0;
    avi_stdindex_header.base_offset[1] = 0;
    avi_stdindex_header.reserved3 = 0;

    // Initialise the standard index entry
    avi_stdindex_entry.offset = 0;
    avi_stdindex_entry.size = 0;

    // Initialise avi index entry
    avi_index_entry.chunk_id.u32 = FCC_00db;
    avi_index_entry.flags = AVIIF_KEYFRAME;
    avi_index_entry.offset = 0;  // Fill in as required
    avi_index_entry.size = 0;  // Fill in later
}


// ------------------------------------------
// Write headers to file
// ------------------------------------------
int32_t c_pipp_avi_write::write_headers()
{
    // Write RIF Header to file
    fwrite (&avi_riff_header , 1 , sizeof(avi_riff_header) , avi_fp);

    // Write hdrl list header to file
    fwrite (&hdrl_list_header , 1 , sizeof(hdrl_list_header) , avi_fp);

    // Write avih chunk header to file
    fwrite (&avih_chunk_header , 1 , sizeof(avih_chunk_header) , avi_fp);

    // Write Main avi header to file
    fwrite (&main_avih_header , 1 , sizeof(main_avih_header) , avi_fp);

    // Write strl list header to file
    fwrite (&strl_list_header , 1 , sizeof(strl_list_header) , avi_fp);

    // Write strh chunk header to file
    fwrite (&strh_chunk_header , 1 , sizeof(strh_chunk_header) , avi_fp);

    // Write video stream header to file
    fwrite (&vids_stream_header , 1 , sizeof(vids_stream_header) , avi_fp);

    // Write strf chunk header to file
    fwrite (&strf_chunk_header , 1 , sizeof(strf_chunk_header) , avi_fp);

    // Write BITMAPINFO header to file
    fwrite (&bitmap_info_header , 1 , sizeof(bitmap_info_header) , avi_fp);

    // Write colour table to file if required (for DIB mono images)
    if (write_colour_table == 1) {
        uint8_t colour_table[256 * 4];
        for (int32_t x = 0; x < 256 * 4; x++) {
            if (x % 4 != 3) {
                colour_table[x] = x / 4;
            } else {
                colour_table[x] = 0;
            }
        }

        // Write colour table to file
        fwrite (colour_table , 1 , 256 * 4, avi_fp);
    }

    if (old_avi_format != 0) {
        // Junk chunk moves next pos to 0x2000
        junk_chunk_header.size = 0x2000 - ftell64(avi_fp) - sizeof(junk_chunk_header);

        // Write junk header to file
        fwrite (&junk_chunk_header , 1 , sizeof(junk_chunk_header) , avi_fp);

        // Write junk data to file
        uint8_t junk_byte = 0;
        for (uint32_t junk_count = 0; junk_count < junk_chunk_header.size; junk_count++) {
            fwrite (&junk_byte , 1 , 1, avi_fp);
        }
    } else {
        // These fields are not present with the old AVI format

        // Write indx chunk header to file
        fwrite (&indx_chunk_header , 1 , sizeof(indx_chunk_header) , avi_fp);

        // Write AVI Superindex header to file
        fwrite (&avi_superindex_header , 1 , sizeof(avi_superindex_header), avi_fp);

        // Write AVI Superindex entries to file
        fwrite (avi_superindex_entries , 1 , sizeof(avi_superindex_entries[0]) * NUMBER_SUPERINDEX_ENTRIES, avi_fp);

        // Write odml list header to file
        fwrite (&odml_list_header , 1 , sizeof(odml_list_header) , avi_fp);

        // Write dmlh chunk header to file
        fwrite (&dmlh_chunk_header , 1 , sizeof(dmlh_chunk_header) , avi_fp);

        // Write extended AVI header to file
        fwrite (&extended_avi_header , 1 , sizeof(extended_avi_header) , avi_fp);
    }

    // Write movi list header
    fwrite (&movi_list_header , 1 , sizeof(movi_list_header) , avi_fp);

    return 0;
}


// ------------------------------------------
// A new frame has been added
// ------------------------------------------
void c_pipp_avi_write::frame_added()
{
    // Split AVI files to prevent max size from being exceeded
    if (old_avi_format != 0) {
        if (current_frame_count == max_frames_in_first_riff) {
            split_close();
            split_create();
        }
    } else {
        // Not old format
        if (riff_count == (NUMBER_SUPERINDEX_ENTRIES-1) && current_frame_count == max_frames_in_other_riffs) {
            split_close();
            split_create();
        }
    }

    if (total_frame_count == 0) {
        // Grab position of first frame in this RIFF for the base offset
        avi_superindex_header.entries_in_use++;
        uint64_t base_offset = ftell64(avi_fp) + sizeof(_00db_chunk_header);
        //uint64_t base_offset = ftell64(avi_fp) - frame_size;
        avi_stdindex_header.base_offset[1] = (uint32_t)(base_offset >> 32);
        avi_stdindex_header.base_offset[0] = (uint32_t)(base_offset & 0xFFFFFFFF);
    }
    
    total_frame_count++;  // Increment frame counts

    if (old_avi_format == 0) {
        if ((riff_count == 0 && current_frame_count == max_frames_in_first_riff)
         || (riff_count > 0 && current_frame_count == max_frames_in_other_riffs)) {
            // This frame needs to be in a new RIFF
            finish_riff();  // Finish this RIFF

            // Start the next RIFF
            fwrite (&avix_riff_header , 1, sizeof(avix_riff_header), avi_fp);

            // Start the new movi LIST
            fwrite (&movi_avix_list_header , 1, sizeof(movi_avix_list_header), avi_fp);

            // Grab position of first frame in this RIFF for the base offset
            avi_superindex_header.entries_in_use++;
            uint64_t base_offset = ftell64(avi_fp) + sizeof(_00db_chunk_header);
            avi_stdindex_header.base_offset[1] = (uint32_t)(base_offset >> 32);
            avi_stdindex_header.base_offset[0] = (uint32_t)(base_offset & 0xFFFFFFFF);
            current_frame_count = 0;
        }
    }

    current_frame_count++;
}


// ------------------------------------------
// Create a new AVI file
// ------------------------------------------
int32_t c_pipp_avi_write::create(
    const char *filename,
    int32_t width,
    int32_t height,
    int32_t colour,
    int32_t fps_rate,
    int32_t fps_scale,
    int32_t old_avi_format,
    int32_t quality __attribute__((unused)),
    void *extra_data __attribute__((unused)))
{
    // Set member variables
    this->width = width;
    this->height = height;
    this->colour = colour;
    this->old_avi_format = old_avi_format;
    if (colour == 0) {
        bytes_per_pixel = 1;
    } else {
        bytes_per_pixel = 3;
    }

    // Call derived class method to set codec specific values
    set_codec_values();

    // Calculate how many frames fit into each RIFF
    if (old_avi_format != 0) {
        // Calculate how many frames can go into the RIFF
        if (old_avi_format == 4) {
            // Max RIFF size = 4GB
            max_frames_in_first_riff = 0xFFFFFFFF;  // Maximum RIFF size (4GB - 1)
        } else {
            // Max RIFF size = 2GB
            max_frames_in_first_riff = 0x7FFFFFFF;  // Maximum RIFF size (2GB - 1)
        }
        //max_frames_in_first_riff -= sizeof(avi_riff_header) ;
        //max_frames_in_first_riff -= (sizeof(hdrl_list_header) - sizeof(hdrl_list_header.four_cc) + hdrl_list_header.size);
        //max_frames_in_first_riff -= (sizeof(junk_chunk_header) + junk_chunk_header.size);
        max_frames_in_first_riff -= 0x2000;  // Junk field always takes us to 0x2000
        max_frames_in_first_riff -= sizeof(movi_list_header);
        max_frames_in_first_riff -= sizeof(idx1_chunk_header);
        max_frames_in_first_riff /= (sizeof(_00db_chunk_header) + frame_size + sizeof(avi_index_entry));
    } else {
        // Calculate how many frames can go into the first RIFF
        max_frames_in_first_riff = 0x3FFFFFFF;  // Maximum RIFF size (1GB - 1)
        max_frames_in_first_riff -= sizeof(avi_riff_header) ;
        max_frames_in_first_riff -= (sizeof(hdrl_list_header) - sizeof(hdrl_list_header.four_cc) + hdrl_list_header.size);
        max_frames_in_first_riff -= sizeof(movi_list_header);
        max_frames_in_first_riff -= (sizeof(ix00_chunk_header) + sizeof(avi_stdindex_header));
        max_frames_in_first_riff -= sizeof(idx1_chunk_header);
        max_frames_in_first_riff /= (sizeof(_00db_chunk_header) + frame_size + sizeof(avi_stdindex_entry) + sizeof(avi_index_entry));

        // Calculate how many frames can go into the subsequent RIFFs
        max_frames_in_other_riffs = 0x7FFFFFFF;  // Maximum RIFF size (2GB - 1)
        max_frames_in_other_riffs -= (sizeof(avix_riff_header) + sizeof(movi_avix_list_header) + sizeof(ix00_chunk_header) + sizeof(avi_stdindex_header));
        max_frames_in_other_riffs /= (sizeof(_00db_chunk_header) + frame_size + sizeof(avi_stdindex_entry));
    }

    split_count = 0;

    this->p_filename = new char[strlen(filename) + 1];
    // Copy filename into buffer
    strcpy(p_filename, filename);

    // Get extension
    char *extension = strrchr(p_filename, '.');
    if (extension == NULL) {
        // No extension found - create one
        this->p_extension = new char[strlen(".avi") + 1];
        strcpy(p_extension, ".avi");  // Copy extension
    } else {
        this->p_extension = new char[strlen(extension) + 1];
        strcpy(p_extension, extension);  // Copy extension
        *extension = 0;  // Remove extension from filename
    }

    // Reset counts
    total_frame_count = 0;
    current_frame_count = 0;
    riff_count = 0;

    // Set flag to write colour table if required
    if (colour == 0) {
        this->write_colour_table = 1;
    }

    // Update AVI structures
    main_avih_header.width = width;
    main_avih_header.height = height;
    uint64_t us_per_frame = (uint64_t)1000000 * (uint64_t)fps_scale;
    us_per_frame /= fps_rate;
    main_avih_header.micro_sec_per_frame = (uint32_t)us_per_frame;
    vids_stream_header.rate = fps_rate;
    vids_stream_header.scale = fps_scale;
    vids_stream_header.frame.right = width;
    vids_stream_header.frame.bottom = height;

    // Reset fields to count frames and indexes
    avi_superindex_header.entries_in_use = 0;  // Will be increment as needed
    extended_avi_header.total_frames = 0;  // Increment as frames are added

    // Set size of strf chunk
    strf_chunk_header.size = sizeof(bitmap_info_header);
    if (write_colour_table == 1) {
        strf_chunk_header.size += 256 * 4;
    }

    // Set size of strl LIST
    strl_list_header.size = sizeof(strl_list_header.four_cc)
                          + sizeof(strh_chunk_header) + strh_chunk_header.size
                          + sizeof(strf_chunk_header) + strf_chunk_header.size;

    if (old_avi_format == 0) {
        strl_list_header.size = strl_list_header.size
                               + sizeof(indx_chunk_header) + indx_chunk_header.size
                               + sizeof(odml_list_header) - sizeof(odml_list_header.four_cc) + odml_list_header.size;
    }
        
    // Set the size of hdrl LIST
    hdrl_list_header.size = sizeof(hdrl_list_header.four_cc)
                          + sizeof(avih_chunk_header) + avih_chunk_header.size
                          + sizeof(strl_list_header) - sizeof(strl_list_header.four_cc) + strl_list_header.size;

    main_avih_header.suggested_buffer_size   = frame_size + 8;
    vids_stream_header.suggested_buffer_size = frame_size + 8;

    bitmap_info_header.size = sizeof(bitmap_info_header);
    bitmap_info_header.width = width;
    bitmap_info_header.height = height;
    bitmap_info_header.bit_count = 8 * bytes_per_pixel;

    // Set colour table length
    if (bytes_per_pixel == 1) {
          bitmap_info_header.clr_used = 256;
    }

    _00db_chunk_header.size = frame_size;

    // Set up MOVI LIST size for AVIX RIFFs as maximum size
    movi_avix_list_header.size = sizeof(movi_avix_list_header.four_cc)
                               + max_frames_in_other_riffs * sizeof(s_chunk_header)       // 00db chunks
                               + max_frames_in_other_riffs * frame_size                   // frame data
                               + sizeof(ix00_chunk_header)                                // ix00 chunk header  
                               + sizeof(avi_stdindex_header)                              // Standard index header
                               + max_frames_in_other_riffs * sizeof(avi_stdindex_entry);  // Standard index entries

    // Set up AVIX RIFF size as maximum size
    avix_riff_header.size = sizeof(avix_riff_header.four_cc)
                          + sizeof(movi_avix_list_header) - sizeof(movi_avix_list_header.four_cc)
                          + movi_avix_list_header.size;

    //::SetWindowTextW(widen(filename).c_str())
    avi_fp = fopen_utf8(filename, "wb+");

    // Free split filename memory
    //delete[] p_split_filename;

    // Check file opened
    // Return if file did not open
    if (!avi_fp) {
        fprintf(stderr, "Error: could not open file '%s' for writing\n", p_filename);
        exit(-1);
    } 

    // Write headers to file
    write_headers();

    // Note that the AVI file is open
    open = 1;

    return 0;
}


// ------------------------------------------
// Create a new AVI file
// ------------------------------------------
int32_t c_pipp_avi_write::split_create()
{
    // Increment split count
    split_count++;

    // Reset counts
    total_frame_count = 0;
    current_frame_count = 0;
    riff_count = 0;

    // Reset fields to count frames and indexes
    avi_superindex_header.entries_in_use = 0;  // Will be increment as needed
    extended_avi_header.total_frames = 0;  // Increment as frames are added

    // Call derived class method to set codec specific values
    set_codec_values();

    // Create split filename
    char *p_split_filename = new char[strlen(p_filename) + 3 + strlen(p_extension) + 1];
    if (split_count == 0) {
        // No split count to be inserted at end of filename
        sprintf(p_split_filename, "%s%s", p_filename, p_extension);
    } else {
        // Split count required at end of filename
        sprintf(p_split_filename, "%s_%02d%s", p_filename, split_count, p_extension);
    }

    // Open new file
    avi_fp = fopen_utf8(p_split_filename, "wb+");

    // Check file opened
    // Return if file did not open
    if (!avi_fp) {
        fprintf(stderr, "Error: could not open file '%s' for writing\n", p_filename);
        exit(-1);
    } 

    // Free split filename memory
    delete[] p_split_filename;

    // Write headers to file
    write_headers();

    return 0;
}


// ------------------------------------------
// Finish the current RIFF
// ------------------------------------------
void c_pipp_avi_write::finish_riff()
{
    int32_t ret;

    // Add odml indexes
    if (old_avi_format == 0) {
        ix00_chunk_header.size = sizeof(avi_stdindex_header)
                               + current_frame_count * sizeof(avi_stdindex_entry);

        // Grab position of the ix00 chunk
        avi_superindex_entries[riff_count].duration = current_frame_count;
        avi_superindex_entries[riff_count].offset = ftell64(avi_fp);
        avi_superindex_entries[riff_count].size = sizeof(ix00_chunk_header) + ix00_chunk_header.size;

        // Write ix00 chunk header to file
        ret = fwrite (&ix00_chunk_header , 1, sizeof(ix00_chunk_header), avi_fp);
        if (ret != sizeof(ix00_chunk_header)) {
            fprintf(stderr, "Error: Error writing to AVI file line %d\n", __LINE__);
            exit(-1);
        }

        // Write AVI standard header to file
        avi_stdindex_header.entries_in_use = current_frame_count;
        ret = fwrite (&avi_stdindex_header , 1, sizeof(avi_stdindex_header), avi_fp);
        if (ret != sizeof(avi_stdindex_header)) {
            fprintf(stderr, "Error: Error writing to AVI file line %d\n", __LINE__);
            exit(-1);
        }

        // Write AVI standard indexes to file
        avi_stdindex_entry.size = frame_size;
        for (uint32_t x = 0; x < current_frame_count; x++) {
            // Update entry
            avi_stdindex_entry.offset = x * (frame_size + sizeof(_00db_chunk_header));


            ret = fwrite (&avi_stdindex_entry , 1, sizeof(avi_stdindex_entry), avi_fp);
            if (ret != sizeof(avi_stdindex_entry)) {
                fprintf(stderr, "Error: Error writing to AVI file line %d\n", __LINE__);
                exit(-1);
            }
        }
    }

    // Update final fields
    extended_avi_header.total_frames = total_frame_count;
    vids_stream_header.length = total_frame_count;

    if (riff_count == 0) {
        // This is the first RIFF - update fields that rely on the first first RIFF

        main_avih_header.total_frames = current_frame_count;

        bitmap_info_header.size_image = frame_size;
        avi_index_entry.size = frame_size;
        idx1_chunk_header.size = current_frame_count * sizeof(s_avi_old_index_entry);
        movi_list_header.size = sizeof(movi_list_header.four_cc)
                              + current_frame_count * sizeof(s_chunk_header)  // 00db chunks
                              + current_frame_count * frame_size;             // frame data

        if (old_avi_format == 0) {
            // Add ix00 index list size on
            movi_list_header.size = movi_list_header.size
                                  + sizeof(ix00_chunk_header)
                                  + ix00_chunk_header.size;
        }

        // Write index chunk header to file
        ret = fwrite (&idx1_chunk_header , 1 , sizeof(idx1_chunk_header), avi_fp);

        if (ret != sizeof(idx1_chunk_header)) {
            fprintf(stderr, "Error: Error writing to AVI file line %d\n", __LINE__);
            exit(-1);
        }

        // Write AVI 1.0 index entries to file
        avi_index_entry.offset = 0x4;

        // Write all entries
        for (uint32_t x = 0; x < current_frame_count; x++) {
            int32_t ret = fwrite (&avi_index_entry , 1 , sizeof(avi_index_entry), avi_fp);  // Write entry to file
            if (ret != sizeof(avi_index_entry)) {
                fprintf(stderr, "Error: Error writing to AVI file line %d\n", __LINE__);
                exit(-1);
            }

            avi_index_entry.offset += (sizeof(s_chunk_header) + frame_size);  // Increment offset
        }

        // Get the filesize
        int64_t filesize = ftell64(avi_fp);

        // Update final headers
        avi_riff_header.size = filesize - 8;
    } 

    // Grab start position of the next RIFF
    int64_t riff_end_position = ftell64(avi_fp);

    // Processing for subsequent RIFFs
    if (riff_count > 0 && current_frame_count != max_frames_in_other_riffs) {
        // This RIFF must be the last RIFF as it does not have the maximum number of frames in it
        // We need to correct the RIFF and LIST sizes as it is not completely full

        // Go back to the start of this RIFF
        fseek64(avi_fp, riff_start_position, SEEK_SET);

        // Write RIFF header again with correct length now that we know it
        avix_riff_header.size = riff_end_position - riff_start_position - sizeof(avix_riff_header) + sizeof(avix_riff_header.four_cc);
        fwrite (&avix_riff_header , 1, sizeof(avix_riff_header), avi_fp);

        // Write the movi LIST header again with the correct length now that we know it
        movi_avix_list_header.size = avix_riff_header.size - sizeof(movi_avix_list_header);
        fwrite (&movi_avix_list_header , 1 , sizeof(movi_avix_list_header) , avi_fp);

        // Go back to the end of this RIFF
        fseek64(avi_fp, riff_end_position, SEEK_SET);
    }

    // Grab start position of the next RIFF
    riff_start_position = riff_end_position;

    // Reset current frame count as this RIFF has been closed
    current_frame_count = 0;

    // Increment RIFF count
    riff_count++;
}



// ------------------------------------------
// Write header and close AVI file
// ------------------------------------------
int32_t c_pipp_avi_write::close()
{
    // Finish off this RIFF
    finish_riff();

    // Go back to start of file
    fseek64(avi_fp, 0, SEEK_SET);

    // Write the updated headers to the file
    write_headers();

    // Note that the AVI file is closed
    open = 0;

    fclose(avi_fp);
    avi_fp = NULL;

    // Free filename buffer
    delete[] p_filename;
    p_filename = NULL;

    // Free extension buffer
    delete[] p_extension;
    p_extension = NULL;

    //debug_headers();

    return 0;
}


// ------------------------------------------
// Write header and close AVI file
// ------------------------------------------
int32_t c_pipp_avi_write::split_close()
{
    // Finish off this RIFF
    finish_riff();

    // Go back to start of file
    fseek64(avi_fp, 0, SEEK_SET);

    // Write the updated headers to the file
    write_headers();

    fclose(avi_fp);
    avi_fp = NULL;

    return 0;
}


int32_t c_pipp_avi_write::debug_headers() {
    // Debug output
    printf("frame_count: %d\n\n", total_frame_count);

    printf("avi_riff_header.list: %c%c%c%c\n",
           avi_riff_header.list.chr[0],
           avi_riff_header.list.chr[1],
           avi_riff_header.list.chr[2],
           avi_riff_header.list.chr[3]);

    printf("avi_riff_header.size: 0x%x\n", avi_riff_header.size);

    printf("avi_riff_header.four_cc: %c%c%c%c\n\n",
           avi_riff_header.four_cc.chr[0],
           avi_riff_header.four_cc.chr[1],
           avi_riff_header.four_cc.chr[2],
           avi_riff_header.four_cc.chr[3]);

    printf("hdrl_list_header.list: %c%c%c%c\n",
           hdrl_list_header.list.chr[0],
           hdrl_list_header.list.chr[1],
           hdrl_list_header.list.chr[2],
           hdrl_list_header.list.chr[3]);

    printf("hdrl_list_header.size: 0x%x\n", hdrl_list_header.size);

    printf("hdrl_list_header.four_cc: %c%c%c%c\n\n",
           hdrl_list_header.four_cc.chr[0],
           hdrl_list_header.four_cc.chr[1],
           hdrl_list_header.four_cc.chr[2],
           hdrl_list_header.four_cc.chr[3]);

    printf("avih_chunk_header.four_cc: %c%c%c%c\n",
           avih_chunk_header.four_cc.chr[0],
           avih_chunk_header.four_cc.chr[1],
           avih_chunk_header.four_cc.chr[2],
           avih_chunk_header.four_cc.chr[3]);

    printf("avih_chunk_header.size: 0x%x\n\n", avih_chunk_header.size);

    printf("main_avih_header.micro_sec_per_frame: %d\n", main_avih_header.micro_sec_per_frame);
    printf("main_avih_header.max_bytes_per_sec: %d\n", main_avih_header.max_bytes_per_sec);
    printf("main_avih_header.padding_granularity: %d\n", main_avih_header.padding_granularity);
    printf("main_avih_header.flags: 0x%x\n", main_avih_header.flags);
    printf("main_avih_header.total_frames: %d\n", main_avih_header.total_frames);
    printf("main_avih_header.initial_frames: %d\n", main_avih_header.initial_frames);
    printf("main_avih_header.streams: %d\n", main_avih_header.streams);
    printf("main_avih_header.suggested_buffer_size: 0x%x\n", main_avih_header.suggested_buffer_size);
    printf("main_avih_header.width: %d\n", main_avih_header.width);
    printf("main_avih_header.height: %d\n\n", main_avih_header.height);

    printf("strl_list_header.list: %c%c%c%c\n",
           strl_list_header.list.chr[0],
           strl_list_header.list.chr[1],
           strl_list_header.list.chr[2],
           strl_list_header.list.chr[3]);

    printf("strl_list_header.size: 0x%x\n", strl_list_header.size);

    printf("strl_list_header.four_cc: %c%c%c%c\n\n",
           strl_list_header.four_cc.chr[0],
           strl_list_header.four_cc.chr[1],
           strl_list_header.four_cc.chr[2],
           strl_list_header.four_cc.chr[3]);

    printf("strh_chunk_header.four_cc: %c%c%c%c\n",
           strh_chunk_header.four_cc.chr[0],
           strh_chunk_header.four_cc.chr[1],
           strh_chunk_header.four_cc.chr[2],
           strh_chunk_header.four_cc.chr[3]);

    printf("strh_chunk_header.size: 0x%x\n", strh_chunk_header.size);

    printf("vids_stream_header.four_cc: %c%c%c%c\n",
           vids_stream_header.handler.chr[0],
           vids_stream_header.handler.chr[1],
           vids_stream_header.handler.chr[2],
           vids_stream_header.handler.chr[3]);

    printf("vids_stream_header.flags: 0x%x\n", vids_stream_header.flags);
    printf("vids_stream_header.priority: 0x%x\n", vids_stream_header.priority);
    printf("vids_stream_header.language: 0x%x\n", vids_stream_header.language);
    printf("vids_stream_header.initial_frames: 0x%x\n", vids_stream_header.initial_frames);
    printf("vids_stream_header.scale: 0x%x\n", vids_stream_header.scale);
    printf("vids_stream_header.rate: %d\n", vids_stream_header.rate);
    printf("vids_stream_header.start: 0x%x\n", vids_stream_header.start);
    printf("vids_stream_header.length: 0x%x\n", vids_stream_header.length);
    printf("vids_stream_header.suggested_buffer_size: 0x%x\n", vids_stream_header.suggested_buffer_size);
    printf("vids_stream_header.quality: 0x%x\n", vids_stream_header.quality);
    printf("vids_stream_header.sample_size: 0x%x\n", vids_stream_header.sample_size);
    printf("vids_stream_header.frame.left: 0x%x\n", vids_stream_header.frame.left);
    printf("vids_stream_header.frame.top: 0x%x\n", vids_stream_header.frame.top);
    printf("vids_stream_header.frame.right: 0x%x\n", vids_stream_header.frame.right);
    printf("vids_stream_header.frame.bottom: 0x%x\n\n", vids_stream_header.frame.bottom);

    printf("strf_chunk_header.four_cc: %c%c%c%c\n",
           strf_chunk_header.four_cc.chr[0],
           strf_chunk_header.four_cc.chr[1],
           strf_chunk_header.four_cc.chr[2],
           strf_chunk_header.four_cc.chr[3]);
    printf("strf_chunk_header.size: 0x%x\n\n", strf_chunk_header.size);

    printf("bitmap_info_header.size: %d\n", bitmap_info_header.size);
    printf("bitmap_info_header.width: %d\n", bitmap_info_header.width);
    printf("bitmap_info_header.height: %d\n", bitmap_info_header.height);
    printf("bitmap_info_header.planes: %d\n", bitmap_info_header.planes);
    printf("bitmap_info_header.bit_count: %d\n", bitmap_info_header.bit_count);
    printf("bitmap_info_header.compression.u32: 0x%x\n", bitmap_info_header.compression.u32);
    printf("bitmap_info_header.size_image: %d\n", bitmap_info_header.size_image);
    printf("bitmap_info_header.x_pels_per_meter: %d\n", bitmap_info_header.x_pels_per_meter);
    printf("bitmap_info_header.y_pels_per_meter: %d\n", bitmap_info_header.y_pels_per_meter);
    printf("bitmap_info_header.clr_used: %d\n", bitmap_info_header.clr_used);
    printf("bitmap_info_header.clr_important: %d\n\n", bitmap_info_header.clr_important);

    printf("movi_list_header.list: %c%c%c%c\n",
           movi_list_header.list.chr[0],
           movi_list_header.list.chr[1],
           movi_list_header.list.chr[2],
           movi_list_header.list.chr[3]);

    printf("movi_list_header.size: 0x%x\n", movi_list_header.size);

    printf("movi_list_header.four_cc: %c%c%c%c\n\n",
           movi_list_header.four_cc.chr[0],
           movi_list_header.four_cc.chr[1],
           movi_list_header.four_cc.chr[2],
           movi_list_header.four_cc.chr[3]);

    printf("_00db_chunk_header.four_cc: %c%c%c%c\n",
           _00db_chunk_header.four_cc.chr[0],
           _00db_chunk_header.four_cc.chr[1],
           _00db_chunk_header.four_cc.chr[2],
           _00db_chunk_header.four_cc.chr[3]);
    printf("_00db_chunk_header.size: 0x%x\n\n", _00db_chunk_header.size);

    printf("idx1_chunk_header.four_cc: %c%c%c%c\n",
           idx1_chunk_header.four_cc.chr[0],
           idx1_chunk_header.four_cc.chr[1],
           idx1_chunk_header.four_cc.chr[2],
           idx1_chunk_header.four_cc.chr[3]);
    printf("idx1_chunk_header.size: 0x%x\n\n", idx1_chunk_header.size);

    printf("avi_index_entry.chunk_id: %c%c%c%c\n",
           avi_index_entry.chunk_id.chr[0],
           avi_index_entry.chunk_id.chr[1],
           avi_index_entry.chunk_id.chr[2],
           avi_index_entry.chunk_id.chr[3]);
    printf("avi_index_entry.flags: 0x%x\n", avi_index_entry.flags);
    printf("avi_index_entry.offset: 0x%x\n", avi_index_entry.offset);
    printf("avi_index_entry.size: %d\n\n", avi_index_entry.size);

    return 0;
}

