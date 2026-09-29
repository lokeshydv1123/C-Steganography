#include <stdio.h>
#include <string.h>
#include "common.h"
#include "encode.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18, and height after that. size is 4 bytes
 */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Seek to the end of the file
    fseek(fptr, 0, SEEK_END);

    // Get the current position in the file, which is the size
    uint size = ftell(fptr);

    // Rewind the file pointer to the beginning of the file
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr;

    /* Check Source Image */
    ptr = strstr(argv[2], ".bmp");

    // Check if the source image has .bmp extension
    if (ptr == NULL){
        printf("ERROR : Source image should be .bmp file\n");
        return e_failure;
    }

    //Check if the image has .bmp extension only
    if (strcmp(ptr, ".bmp") == 0)
        encInfo->src_image_fname = argv[2];
    else{
        printf("ERROR : Invalid Source Image\n");
        return e_failure;
    }

    // Check if the secret file has .txt
    if ((ptr = strstr(argv[3], ".txt")) != NULL){
        if (strcmp(ptr, ".txt") == 0)
            encInfo->secret_fname = argv[3];
        else{
            printf("ERROR : Secret file should be .txt, .c or .sh\n");
            return e_failure;
        }
    }

    // Check if the secret file has .c
    else if ((ptr = strstr(argv[3], ".c")) != NULL){
        if (strcmp(ptr, ".c") == 0)
            encInfo->secret_fname = argv[3];
        else{
            printf("ERROR : Secret file should be .txt, .c or .sh\n");
            return e_failure;
        }
    }

    // Check if the secret file has .sh

    else if ((ptr = strstr(argv[3], ".sh")) != NULL){
        if (strcmp(ptr, ".sh") == 0)
            encInfo->secret_fname = argv[3];
        else{
            printf("ERROR : Secret file should be .txt, .c or .sh\n");
            return e_failure;
        }
    }

    /* Destination Image */
    if (argv[4] != NULL){
        ptr = strstr(argv[4], ".bmp");

        // Check if the destination image has .bmp extension
        if (ptr == NULL){
            printf("ERROR : Output image should be .bmp file\n");
            return e_failure;
        }

        //Check if the destination image has .bmp extension only
        if (strcmp(ptr, ".bmp") == 0)
            encInfo->dest_image_fname = argv[4];
        else{
            printf("ERROR : Invalid Output Image\n");
            return e_failure;
        }
    }
    else
        // If destination image is not provided, use default name
        encInfo->dest_image_fname = "default.bmp";

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");

    // Check if source image is opened successfully
    if (encInfo->fptr_src_image == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    // Check if secret file is opened successfully
    if (encInfo->fptr_secret == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "wb");

    // Check if destination file is opened successfully
    if (encInfo->fptr_dest_image == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// Function to check if the source image has enough capacity to hold the secret data
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if (encInfo->image_capacity > (16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8)))
        return e_success;
    else
        return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char image_buffer[54];

    // Read the 54-byte header from the source image and write it to the destination image
    fread(image_buffer, 54, 1, fptr_src_image);
    fwrite(image_buffer, 54, 1, fptr_dest_image);

    // Check if the file pointers are at the correct position after copying the header
    if (ftell(fptr_src_image) == 54 && ftell(fptr_dest_image) == 54)
        return e_success;

    return e_failure;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];

    // Encode the magic string into the image by encoding each character into the least significant bits of the image buffer
    for (int i = 0; i < strlen(magic_string); i++){
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], image_buffer);
        fwrite(image_buffer, 8, 1, encInfo->fptr_dest_image);
    }

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
        return e_success;

    return e_failure;
}

// Function to encode the size of the secret file extension into the image
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char image_buffer[32];

    // Read the size of the secret file extension from the source image and encode it into the least significant bits of the image buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, image_buffer);
    fwrite(image_buffer, 32, 1, encInfo->fptr_dest_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
        return e_success;

    return e_failure;
}

// Function to encode the secret file extension into the image
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char image_buffer[8];

    // Loop to encode each character of the secret file extension into the least significant bits of the image buffer
    for (int i = 0; i < strlen(file_extn); i++){
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], image_buffer);
        fwrite(image_buffer, 8, 1, encInfo->fptr_dest_image);
    }

    // Check if the file pointers are at the correct position after encoding the secret file extension
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
        return e_success;

    return e_failure;
}

// Function to encode the size of the secret file into the image
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];

    // Read the size of the secret file from the source image and encode it into the least significant bits of the image buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, image_buffer);
    fwrite(image_buffer, 32, 1, encInfo->fptr_dest_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
        return e_success;

    return e_failure;
}

// Function to encode the secret file data into the image
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Read the secret file data from the secret file and encode it into the least significant bits of the image buffer
    rewind(encInfo->fptr_secret);
    char file_data[encInfo->size_secret_file];
    fread(file_data, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    char image_buffer[8];

    // Loop to encode each byte of the secret file data into the least significant bits of the image buffer
    for (int i = 0; i < encInfo->size_secret_file; i++){
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_data[i], image_buffer);
        fwrite(image_buffer, 8, 1, encInfo->fptr_dest_image);
    }

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
        return e_success;

    return e_failure;
}

// Function to encode a single byte into the least significant bits of the image buffer
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Loop to encode each bit of the byte into the least significant bits of the image buffer
    for (int i = 0; i < 8; i++){
        char bit = (data >> (7 - i)) & 1;
        image_buffer[i] &= 0xFE;
        image_buffer[i] |= bit;
    }

    return e_success;
}

// Function to encode size into the least significant bits of the image buffer
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // Loop to encode each bit of the size into the least significant bits of the image buffer
    for (int i = 0; i < 32; i++){
        int bit = (size >> (31 - i)) & 1;
        imageBuffer[i] &= 0xFE;
        imageBuffer[i] |= bit;
    }

    return e_success;
}

// Function to copy the remaining image data from the source image to the destination image
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    // Loop to read each byte from the source image and write it to the destination image
    while (fread(&ch, 1, 1, fptr_src) > 0)
        fwrite(&ch, 1, 1, fptr_dest);

    return e_success;
}

// Function to perform the encoding process
Status do_encoding(EncodeInfo *encInfo)
{
    // Step 1: Open the source image, secret file, and destination image files
    if (open_files(encInfo) == e_success)
        printf("INFO : Files opened successfully\n");
    else{
        printf("ERROR : Failed to open files\n");
        return e_failure;
    }

    // Step 2: Check if the source image has enough capacity to hold the secret data
    if (check_capacity(encInfo) == e_success)
        printf("INFO : Capacity check successful\n");
    else{
        printf("ERROR : Insufficient image capacity\n");
        return e_failure;
    }

    // Step 3: Copy the BMP header from the source image to the destination image
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_dest_image) == e_success)
        printf("INFO : BMP Header copied successfully\n");
    else{
        printf("ERROR : Failed to copy BMP Header\n");
        return e_failure;
    }

    // Step 4: Encode the magic string into the image to indicate that it contains hidden data
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
        printf("INFO : Magic String Encoded Successfully\n");
    else{
        printf("ERROR : Failed to Encode Magic String\n");
        return e_failure;
    }

    // Step 5: Encode the secret file extension into the image
    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));

    // Step 6: Encode the size of the secret file extension into the image
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
        printf("INFO : Secret File Extension Size Encoded Successfully\n");
    else{
        printf("ERROR : Failed to Encode Secret File Extension Size\n");
        return e_failure;
    }

    // Step 7: Encode the secret file extension into the image
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
        printf("INFO : Secret File Extension Encoded Successfully\n");
    else{
        printf("ERROR : Failed to Encode Secret File Extension\n");
        return e_failure;
    }

    // Step 8 : Encode the size of the secret file into the image
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
        printf("INFO : Secret File Size Encoded Successfully\n");
    else{
        printf("ERROR : Failed to Encode Secret File Size\n");
        return e_failure;
    }

    // Step 9 : Encode the secret file data into the image
    if (encode_secret_file_data(encInfo) == e_success)
        printf("INFO : Secret File Data Encoded Successfully\n");
    else{
        printf("ERROR : Failed to Encode Secret File Data\n");
        return e_failure;
    }

    // Step 10: Copy the remaining image data from the source image to the destination image
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_dest_image) == e_success){
        printf("INFO : Remaining Image Data Copied Successfully\n");
        return e_success;
    }
    else{
        printf("ERROR : Failed to Copy Remaining Image Data\n");
        return e_failure;
    }
}