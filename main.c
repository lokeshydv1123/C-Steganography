/* Name - Lokesh Yadav

 * Project 2 - Stegonography Project

 * Encode.c - This file contains the implementation of the encoding functionality for the steganography project.

 * It includes functions to read and validate command line arguments, check image capacity, copy BMP headers, encode magic strings, 
   secret file extensions, sizes, and data into the source image.
 
 * Secret.txt - This file contains the secret data that will be encoded into the source image.
 
 * Common.h - This header file contains common definitions and declarations used across the project.
 
 * Encode.h - This header file contains function declarations and data structures related to the encoding functionality.
 
 * Beautiful.bmp - This is the source image file that will be used for encoding the secret data.
 
 * Decode.c - This file contains the implementation of the decoding functionality for the steganography project.
 
 * Decode.h - This header file contains function declarations and data structures related to the decoding functionality.
 
 * main.c - This file contains the main function that serves as the entry point for the steganography project. 
   It handles command line arguments, determines the operation type (encoding or decoding), and invokes the appropriate functions for encoding or
   decoding based on user input.
 
 * types.h - This header file contains type definitions and enumerations used throughout the project.

 * default.bmp - This is the default image file that will be used for encoding the secret data if no output image is specified by the user.

 * output.txt - This is the output file that will be generated after decoding the secret data from the stego image.
 */

#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    OperationType op;

    // Check whether operation argument is present 
    if (argc < 2)
    {
        printf("ERROR : Insufficient Arguments\n\n");

        printf("Encoding Usage:\n");
        printf("./a.out -e <input.bmp> <secret.txt/.c/.sh> [output.bmp]\n\n");

        printf("Decoding Usage:\n");
        printf("./a.out -d <stego.bmp> [output_file]\n");

        return e_failure;
    }

    /* Check operation type */
    op = check_operation_type(argv[1]);

    switch(op)
    {
        // Check if the operation type is encoding
        case e_encode:
        {
            // Encoding requires at least 4 arguments 
            if (argc < 4)
            {
                printf("ERROR : Insufficient Arguments for Encoding\n");
                printf("Usage : ./a.out -e <input.bmp> <secret.txt/.c/.sh> [output.bmp]\n");
                return e_failure;
            }

            printf("INFO : User selected Encoding\n");

            EncodeInfo encInfo;

            // Calling the read and validate function to check the validity of the input arguments
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                printf("INFO : Read and Validate Success\n");

                if (do_encoding(&encInfo) == e_success)
                    printf("INFO : Encoding completed successfully\n");
                else{
                    printf("ERROR : Encoding Failed\n");
                    return e_failure;
                }
            }

            // If the read and validate function fails, print an error message and return failure
            else
            {
                printf("ERROR : Read and Validate Failed\n");
                return e_failure;
            }

            break;
        }

        // Check if the operation type is decoding
        case e_decode:
        {
            /* Decoding requires at least 3 arguments */
            if (argc < 3)
            {
                printf("ERROR : Insufficient Arguments for Decoding\n");
                printf("Usage : ./a.out -d <stego.bmp> [output_file]\n");
                return e_failure;
            }

            printf("INFO : User selected Decoding\n");

            DecodeInfo decInfo;

            // Calling the read and validate function to check the validity of the input arguments
            if (read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                printf("INFO : Read and Validate Success\n");

                if (do_decoding(&decInfo) == e_success)
                    printf("INFO : Decoding completed successfully\n");
                else{
                    printf("ERROR : Decoding Failed\n");
                    return e_failure;
                }
            }

            else
            {
                printf("ERROR : Read and Validate Failed\n");
                return e_failure;
            }

            break;
        }

        default:
        {
            printf("ERROR : Unsupported Operation\n");
            printf("Use -e for Encoding\n");
            printf("Use -d for Decoding\n");

            return e_failure;
        }
    }

    return e_success;
}

// Function to check operation type 

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
        return e_encode;
    
    else if (strcmp(symbol, "-d") == 0)
        return e_decode;
    
    else
        return e_unsupported;
}