#ifndef PIPP_BUFFER_H
#define PIPP_BUFFER_H

// #include <cstdint>
#include <stdint.h>
#include <cstddef>


//#define DEBUG


class c_pipp_buffer {
    // ------------------------------------------
    // Private definitions
    // ------------------------------------------
    private:
        uint32_t buffer_size;
        uint8_t *buffer;

    // ------------------------------------------
    // Public definitions
    // ------------------------------------------

    public:
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_pipp_buffer() : 
            buffer_size(0),
            buffer(nullptr) {};


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        ~c_pipp_buffer() {
            delete [] buffer;
        };


        // ------------------------------------------
        // Get buffer ptr
        // ------------------------------------------
        inline uint8_t *get_buffer_ptr() {
            return buffer;
        };


        // ------------------------------------------
        // Member function to set a new  buffer
        // ------------------------------------------
        // Caller has the responsibility to esure the old buffer is freed
        // and the new buffer is large enough
        uint8_t *set_buffer_ptr(uint8_t *new_ptr) {
            uint8_t *ret = buffer;
            buffer = new_ptr;
            return ret;
        }


        // ------------------------------------------
        // Member function to get a new  buffer
        // ------------------------------------------
        uint8_t *get_buffer(
            uint32_t size);


        // ------------------------------------------
        // Member function to get a new zeroed buffer
        // ------------------------------------------
        uint8_t *get_zero_buffer(
            uint32_t size);


        // ------------------------------------------
        // Member function to delete buffer
        // ------------------------------------------
        void delete_buffer();

};


#endif  // PIPP_BUFFER_H
