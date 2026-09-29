#include <stdio.h>
#include <string.h>

#include "decode.h"
#include "common.h"
#include "types.h"

// Read and Validate Decode Arguments

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ptr;

    // Check the stego image file extension
    ptr = strstr(argv[2], ".bmp");

    if (ptr == NULL){
        printf("ERROR : Stego image should be .bmp file\n");
        return e_failure;
    }

    // Check if it is a valid stego image file .bmp
    if (strcmp(ptr, ".bmp") == 0)
        decInfo->stego_image_fname = argv[2];
    else{
        printf("ERROR : Invalid Stego Image\n");
        return e_failure;
    }

    // Check the output file name if provided
    if (argv[3] != NULL)
        decInfo->output_fname = argv[3];
    else
        decInfo->output_fname = "output";

    return e_success;
}

// Function to open the stego image file for decoding
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");

    // Check if the stego image file was opened successfully
    if (decInfo->fptr_stego_image == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// Function to decode a byte from the least significant bits of the image buffer
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;

    // Loop to decode a byte from the least significant bits of the image buffer
    for(int i = 0; i < 8; i++){
        data = data << 1;
        data |= (image_buffer[i] & 1);
    }

    return data;
}

// Function to decode the size of the secret file from the least significant bits of the image buffer
int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;

    // Loop to decode the size of the secret file from the least significant bits of the image buffer
    for(int i = 0; i < 32; i++){
        size = size << 1;
        size |= (image_buffer[i] & 1);
    }

    return size;
}

// Function to decode the magic string from the image and validate it
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_string[strlen(MAGIC_STRING) + 1];

    // Loop to decode each byte of the magic string and store it in the magic_string array
    for(int i = 0; i < strlen(MAGIC_STRING); i++){
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        magic_string[i] = decode_byte_from_lsb(image_buffer);
    }

    magic_string[strlen(MAGIC_STRING)] = '\0';

    // Compare the decoded magic string with the expected MAGIC_STRING
    if(strcmp(magic_string, MAGIC_STRING) == 0)
        return e_success;

    return e_failure;
}

// Function to decode the size of the secret file extension from the image
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    // Read the size of the secret file extension from the image and store it in the DecodeInfo structure
    char image_buffer[32];
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->extn_size = decode_size_from_lsb(image_buffer);

    return e_success;
}

// Function to decode the secret file extension from the image and create the output file
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char output_file[50];
    char *token;

    // Loop to decode each byte of the secret file extension
    for(int i = 0; i < decInfo->extn_size; i++){
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    // Copy the output filename
    strcpy(output_file, decInfo->output_fname);

    // Remove the existing extension (if any)
    token = strtok(output_file, ".");

    if(token != NULL)
        strcpy(output_file, token);

    // Append the decoded extension
    strcat(output_file, decInfo->extn_secret_file);

    // Open the output file
    decInfo->fptr_output = fopen(output_file, "wb");

    if(decInfo->fptr_output == NULL){
        perror("fopen");
        return e_failure;
    }

    return e_success;
}

// Function to decode the size of the secret file from the image
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    // Read the size of the secret file from the image and store it in the DecodeInfo structure
    char image_buffer[32];
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->secret_file_size = decode_size_from_lsb(image_buffer);

    return e_success;
}

// Function to decode the secret file data from the image and write it to the output file
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    // Loop to decode each byte of the secret file data and write it to the output file
    for(int i = 0; i < decInfo->secret_file_size; i++){
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        ch = decode_byte_from_lsb(image_buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}

// Function to perform the decoding process
Status do_decoding(DecodeInfo *decInfo)
{
    // Step 1: Open the stego image file and prepare for decoding
    if(open_decode_files(decInfo) == e_success)
        printf("INFO : Files opened successfully\n");
    else{
        printf("ERROR : Failed to open files\n");
        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // Step 2: Decode the magic string and validate it
    if(decode_magic_string(decInfo) == e_success)
        printf("INFO : Magic String Decoded Successfully\n");
    else{
        printf("ERROR : Magic String Mismatch\n");
        return e_failure;
    }

    // Step 3: Decode the size of the secret file extension
    if(decode_secret_file_extn_size(decInfo) == e_success)
        printf("INFO : Extension Size Decoded Successfully\n");
    else{
        printf("ERROR : Failed to Decode Extension Size\n");
        return e_failure;
    }

    // Step 4: Decode the secret file extension and create the output file
    if(decode_secret_file_extn(decInfo) == e_success)
        printf("INFO : Extension Decoded Successfully\n");
    else{
        printf("ERROR : Failed to Decode Extension\n");
        return e_failure;
    }

    // Step 5: Decode the size of the secret file
    if(decode_secret_file_size(decInfo) == e_success)
        printf("INFO : Secret File Size Decoded Successfully\n");
    else{
        printf("ERROR : Failed to Decode Secret File Size\n");
        return e_failure;
    }

    // Step 6: Decode the secret file data and write it to the output file
    if(decode_secret_file_data(decInfo) == e_success)
        printf("INFO : Secret File Data Decoded Successfully\n");
    else{
        printf("ERROR : Failed to Decode Secret File Data\n");
        return e_failure;
    }

    // Step 7: Close the file pointers after decoding is complete
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}