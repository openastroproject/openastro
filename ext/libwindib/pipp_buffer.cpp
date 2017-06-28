#include <cstring>
#include <cstdlib>
//#include <cstdint>
#include <iostream>
#include "pipp_buffer.h"


using namespace std;


// ------------------------------------------
// Member function to get a new buffer
// ------------------------------------------
uint8_t *c_pipp_buffer::get_buffer(
    uint32_t size)
{
    if (size > buffer_size) {
        // The current buffer is not big enough
        // Delete the current one and generate a new one that is large enough
        delete [] buffer;

        // Calculate buffer size
        buffer_size = 128;
        while (buffer_size < size) {
            buffer_size *= 2;
        }

        // Create the buffer
        try {
            buffer = new uint8_t[buffer_size];
        } catch(...) {
            cout << "FATAL ERROR: memory allocation (";
            cout << dec << (size * sizeof(buffer_size));
            cout << ") failed in c_pipp_buffer::get_buffer()" << endl;
            exit(-1);
        }
    }

    // Return pointer to buffer
    return buffer;
}


// ------------------------------------------
// Member function to get a new zeroed buffer
// ------------------------------------------
uint8_t *c_pipp_buffer::get_zero_buffer(
    uint32_t size)
{
    // Call get_buffer() method to create buffer as usual
    get_buffer(size);

    // Zero the buffer
    memset(buffer, 0, buffer_size);

    // Return pointer to buffer
    return buffer;
}


// ------------------------------------------
// Member function to delete buffer
// ------------------------------------------
void c_pipp_buffer::delete_buffer() {
    delete [] buffer;
    buffer = NULL;
    buffer_size = 0;
}
