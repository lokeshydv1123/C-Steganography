#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

// Structure to store information required for decoding secret file from stego image.
typedef struct _DecodeInfo
{
    char *stego_image_fname;       // To store stego image name
    FILE *fptr_stego_image;        // To store stego image file pointer

    char *output_fname;            // Output filename (without extension)
    FILE *fptr_output;             // Output file pointer

    char extn_secret_file[20];     // Decoded extension (.txt, .pdf, etc.)
    int extn_size;                 // Extension size
    long secret_file_size;         // Secret file size

} DecodeInfo;

/* Read and Validate Decode Arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform Decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open Stego Image */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode Secret File Extension Size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode Secret File Extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode Secret File Size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode Secret File Data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode One Byte From LSB */
char decode_byte_from_lsb(char *image_buffer);

/* Decode Integer From LSB */
int decode_size_from_lsb(char *image_buffer);

#endif