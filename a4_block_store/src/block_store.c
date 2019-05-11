#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "bitmap.h"
#include "block_store.h"
#include <errno.h>

#define BLOCK_STORE_NUM_BLOCKS 256   // 2^8 blocks.
#define BLOCK_STORE_AVAIL_BLOCKS 255 // First block consumed by the FBM
#define BLOCK_STORE_NUM_BYTES 65536  // 2^8 blocks of 2^8 bytes.
#define BLOCK_SIZE_BYTES 256         // 2^8 BYTES per block

//the block_store struct, contains the bitmap fbm to keep track of available and used blocks
//can store 2^8 blocks of 2^8 bytes, the first block being the fbm
//fbm is physically stored in the first block of the blocks array, with a pointer to keep track of it in the struct
struct block_store {
    void* blocks;
    bitmap_t* fbm;
};

///
/// This creates a new BS device, ready to go
/// \return Pointer to a new block storage device, NULL on error
///
block_store_t *block_store_create() {
    //malloc struct
    block_store_t* bs = malloc(sizeof(block_store_t));

    //check for allocation errors
    if(bs==NULL) {
        return NULL;
    }

    //allocate and zero out the block store (calloc'ed in order to init bitmap as all zeros)
    bs->blocks = calloc(BLOCK_STORE_NUM_BLOCKS, BLOCK_SIZE_BYTES);

    //check for allocation errors
    if(bs->blocks==NULL) {
        block_store_destroy(bs);
        return NULL;
    }

    //use the first block as the fbm and init it
    bs->fbm = bitmap_overlay(BLOCK_STORE_AVAIL_BLOCKS, bs->blocks);

    return bs;
}

///
/// Destroys the provided block storage device
/// This is an idempotent operation, so there is no return value
/// \param bs BS device
///
void block_store_destroy(block_store_t *const bs) {
    //check that bs is valid
    if(bs == NULL) {
        return;
    }
    else {
        //free inner objects then the whole struct
        
        if(bs->blocks != NULL) {
            free(bs->blocks);

        }

        if(bs->fbm != NULL) {
            free(bs->fbm);
        }

        free(bs);
    }
}

///
/// Searches for a free block, marks it as in use, and returns the block's id
/// \param bs BS device
/// \return Allocated block's id, SIZE_MAX on error
///
size_t block_store_allocate(block_store_t *const bs) {
    //check that bs is valid
    if(bs==NULL) {
        return SIZE_MAX;
    }

    //find the first free (zero) by using the fbm
    size_t firstFree = 0;
    firstFree = bitmap_ffz(bs->fbm);
    
    //make sure that there is an open slot and no error returned
    if(firstFree == SIZE_MAX) {
        return firstFree;
    }
    
    //mark the block as in use in the fbm
    bitmap_set(bs->fbm, firstFree);

    return firstFree;
}

///
/// Attempts to allocate the requested block id
/// \param bs the block store object
/// \block_id the requested block identifier
/// \return boolean indicating succes of operation
///
bool block_store_request(block_store_t *const bs, const size_t block_id) {
    //make sure that bs and block_id are valid
    if(bs == NULL || block_id>block_store_get_total_blocks()) {
        return false;
    }

    //check to see if the requested block_id is set in the fbm
    bool isSet = false;
    isSet = bitmap_test(bs->fbm, block_id);

    //if it is not set, then mark set and return that it can be used, else false - it is already in use
    if(!isSet) {
        bitmap_set(bs->fbm, block_id);
        return true;
    }
    else {
        return false;
    }
}

///
/// Frees the specified block
/// \param bs BS device
/// \param block_id The block to free
///
void block_store_release(block_store_t *const bs, const size_t block_id) {
    //check that bs and block_id are valid
    if(bs==NULL || block_id>block_store_get_total_blocks()) {
        return;
    }
    //release (zero out) the given block_id in the fbm
    bitmap_reset(bs->fbm, block_id);
}

///
/// Counts the number of blocks marked as in use
/// \param bs BS device
/// \return Total blocks in use, SIZE_MAX on error
///
size_t block_store_get_used_blocks(const block_store_t *const bs) {
    //make sure that bs is valid
    if(bs==NULL) {
        return SIZE_MAX;
    }

    //get and return amount of total blocks that are in use (set bits)
    size_t totalSet = bitmap_total_set(bs->fbm);

    return totalSet;
}

///
/// Counts the number of blocks marked free for use
/// \param bs BS device
/// \return Total blocks free, SIZE_MAX on error
///
size_t block_store_get_free_blocks(const block_store_t *const bs) {
    //check to make sure bs is valid
    if(bs == NULL) {
        return SIZE_MAX;
    }

    //total bits in fbm - total set in fbm = total free in fbm
    size_t totalFree = bitmap_get_bits(bs->fbm) - bitmap_total_set(bs->fbm);

    return totalFree;
}

///
/// Returns the total number of user-addressable blocks
///  (since this is constant, you don't even need the bs object)
/// \return Total blocks
///
size_t block_store_get_total_blocks() {
    return BLOCK_STORE_AVAIL_BLOCKS;
}

///
/// Reads data from the specified block and writes it to the designated buffer
/// \param bs BS device
/// \param block_id Source block id
/// \param buffer Data buffer to write to
/// \return Number of bytes read, 0 on error
///
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer) {
    //check to make sure bs, buffer, and block_id are valid
    if(bs==NULL || buffer==NULL || block_id>block_store_get_total_blocks()) {
        return 0;
    }

    //make sure this block is actually in use
    if(bitmap_test(bs->fbm, block_id)==0) {
        return 0;
    }
    else {
        //copy contents from specified block into buffer
        memcpy(buffer, (bs->blocks+(block_id*BLOCK_SIZE_BYTES)), BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }
}

///
/// Reads data from the specified buffer and writes it to the designated block
/// \param bs BS device
/// \param block_id Destination block id
/// \param buffer Data buffer to read from
/// \return Number of bytes written, 0 on error
///
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer) {
    //validate bs, buffer, block_id
    if(bs==NULL || buffer==NULL || block_id>block_store_get_total_blocks()) {
        return 0;
    }

    //make sure that the block has been requested first and can be written to
    if(bitmap_test(bs->fbm, block_id)==0) {
        return 0;
    }
    else {
        //copy contents from buffer into the proper id in the block array
        memcpy((bs->blocks+(block_id*BLOCK_SIZE_BYTES)), buffer, BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }
}

///
/// Imports BS device from the given file - for grads/bonus
/// \param filename The file to load
/// \return Pointer to new BS device, NULL on error
///
block_store_t *block_store_deserialize(const char *const filename) {
    //make sure filename is valid
    if(filename==NULL) {
        return NULL;
    }

    //open file
    int fd = open(filename, O_RDONLY);

    //make sure file opening was a success
    if(fd<0) {
        return NULL;
    }

    //create the bs into which the deserialized data will go
    block_store_t* bs = block_store_create();

    //read from the file and put the data into blocks array
    size_t bytes = read(fd, bs->blocks, BLOCK_STORE_NUM_BYTES);

    //make sure any data was read
    if(bytes==0) {
        block_store_destroy(bs);
        return NULL;
    }

    return bs;
}

///
/// Writes the entirety of the BS device to file, overwriting it if it exists - for grads/bonus
/// \param bs BS device
/// \param filename The file to write to
/// \return Number of bytes written, 0 on error
///
size_t block_store_serialize(const block_store_t *const bs, const char *const filename) {
    //make sure bs and filename are valid
    if(bs==NULL || filename==NULL) {
        return 0;
    }

    //open file in create mode to start in case it has not been created yet
    //source: https://stackoverflow.com/questions/15798450/open-with-o-creat-was-it-opened-or-created
    int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0644);
    
    //if error or file already exists, now open in write mode
    if ((fd == -1) && (errno == EEXIST))
    {
        fd = open(filename, O_WRONLY);
    }

    //if all file opening fails, return error
    if(fd<0) {
        return 0;
    }

    //write blockstore into file and return the number of successful bytes written
    size_t bytes = write(fd, bs->blocks, BLOCK_STORE_NUM_BYTES);

    //make things actually written into file
    if(bytes==0) {
        return 0;
    }

    return bytes;
}
