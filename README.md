# C Steganography

A C based image steganography project that hides secret file data inside a BMP image using the Least Significant Bit (LSB) technique.

The project supports both **encoding** secret data into an image and **decoding** the hidden data from a stego image.

## Overview

Steganography is the technique of hiding information inside another medium so that the existence of the hidden information is not obvious.

In this project, secret data is embedded inside the pixel data of a BMP image by modifying the **least significant bits** of image bytes.

The original BMP header is preserved, while the secret information is stored in the image data.

## Features

* Encode secret data into a BMP image
* Decode hidden data from a stego BMP image
* LSB based data hiding
* BMP image capacity checking
* Magic string validation
* Store and recover secret file extension
* Store and recover secret file size
* Support for `.txt`, `.c`, and `.sh` secret files
* Command line interface
* Error handling for invalid files and insufficient arguments
* Default output image and output file names

## How It Works

The encoding process stores information in the following order:

```text
BMP Header
     ↓
Magic String
     ↓
Secret File Extension Size
     ↓
Secret File Extension
     ↓
Secret File Size
     ↓
Secret File Data
     ↓
Remaining Image Data
```

The decoder reads the information in the same order and reconstructs the original secret file.

## LSB Technique

The Least Significant Bit of an image byte is used to store one bit of secret information.

For example:

```text
Original image byte:

10110110

Secret bit:

        1

Modified image byte:

10110111
```

Only the least significant bit is modified, so the change to the image data is minimal.

For every secret byte, the project reads 8 image bytes and stores one bit of the secret byte in each image byte.

```c
for (int i = 0; i < 8; i++)
{
    char bit = (data >> (7 - i)) & 1;
    image_buffer[i] &= 0xFE;
    image_buffer[i] |= bit;
}
```

The decoder reverses this process by reading the LSB from 8 image bytes and reconstructing the original byte.

## Encoding Process

The encoding process performs the following steps:

1. Validate command line arguments.
2. Open the source BMP image.
3. Open the secret file.
4. Create the destination stego image.
5. Check whether the image has sufficient capacity.
6. Copy the 54 byte BMP header.
7. Encode the magic string.
8. Encode the secret file extension size.
9. Encode the secret file extension.
10. Encode the secret file size.
11. Encode the secret file data.
12. Copy the remaining image data.

The implementation performs these operations sequentially in `do_encoding()`.

## Decoding Process

The decoding process performs the following steps:

1. Validate the stego image argument.
2. Open the stego image.
3. Skip the 54 byte BMP header.
4. Decode and validate the magic string.
5. Decode the secret file extension size.
6. Decode the secret file extension.
7. Create the output file using the recovered extension.
8. Decode the secret file size.
9. Decode the secret file data.
10. Write the recovered data into the output file.
11. Close the files.

## Magic String

The project uses a magic string to identify whether the image contains encoded data.

```c
#define MAGIC_STRING "#*"
```

During decoding, the program extracts the stored magic string and compares it with the expected value.

If the values do not match, decoding is stopped.

## Capacity Checking

Before encoding, the program calculates the image capacity using:

```text
width × height × 3
```

The project assumes 3 bytes per pixel for the BMP image.

It then checks whether the image has enough capacity for:

```text
Magic String
+ Extension Size
+ Extension
+ Secret File Size
+ Secret File Data
```

Encoding is stopped if the image does not have sufficient capacity.

## Project Structure

```text
C-Steganography/
│
├── main.c
├── encode.c
├── encode.h
├── decode.c
├── decode.h
├── common.h
├── types.h
├── secret.txt
└── beautiful.bmp
```

### File Description

| File            | Description                                     |
| --------------- | ----------------------------------------------- |
| `main.c`        | Entry point and command line operation handling |
| `encode.c`      | Encoding implementation                         |
| `encode.h`      | Encoding structures and function declarations   |
| `decode.c`      | Decoding implementation                         |
| `decode.h`      | Decoding structures and function declarations   |
| `common.h`      | Common definitions including the magic string   |
| `types.h`       | User defined types and operation/status enums   |
| `secret.txt`    | Sample secret data                              |
| `beautiful.bmp` | Sample BMP image                                |

The `EncodeInfo` structure stores source image, secret file, and destination image information. The decoder similarly stores the stego image, output file, extension and secret file size in `DecodeInfo`.

## Compilation

Compile the project using GCC:

```bash
gcc main.c encode.c decode.c -o steganography
```

## Usage

### Encoding

```bash
./steganography -e <input.bmp> <secret_file> [output.bmp]
```

Example:

```bash
./steganography -e beautiful.bmp secret.txt stego.bmp
```

If the output image name is not provided, the program uses:

```text
default.bmp
```

### Decoding

```bash
./steganography -d <stego.bmp> [output_file]
```

Example:

```bash
./steganography -d stego.bmp decoded
```

The decoded file extension is recovered from the information stored inside the image.

## Example Workflow

### Step 1: Encode

```bash
./steganography -e beautiful.bmp secret.txt stego.bmp
```

The program embeds the contents of `secret.txt` inside `beautiful.bmp` and generates:

```text
stego.bmp
```

### Step 2: Decode

```bash
./steganography -d stego.bmp decoded
```

The hidden data is extracted and written to the output file with the recovered extension.

## Technologies Used

* C
* File Handling
* Structures
* Pointers
* Command Line Arguments
* Bitwise Operations
* BMP File Handling
* Least Significant Bit Encoding
* Modular Programming

## Key Concepts Demonstrated

This project demonstrates practical usage of:

* `fopen()`
* `fread()`
* `fwrite()`
* `fseek()`
* `ftell()`
* `rewind()`
* `fclose()`
* `strcmp()`
* `strstr()`
* `strcpy()`
* `strcat()`
* Bit shifting
* Bit masking
* Structures
* Enumerations
* Header files
* Function modularization

## Limitations

* The current implementation is designed for BMP images.
* The implementation assumes 3 bytes per pixel when calculating image capacity.
* The project provides steganography functionality but does not provide encryption of the hidden data.
* The hidden information is therefore not equivalent to cryptographic protection.

## Future Improvements

Possible improvements include:

* Add encryption before embedding secret data.
* Support additional image formats.
* Improve input validation.
* Add dynamic memory management for larger secret files.
* Add a Makefile for easier compilation.
* Improve error handling and resource cleanup.
* Add automated tests for encoding and decoding.

## Author

**Lokesh Yadav**

B.Tech Computer Science and Engineering

## License

This project is intended for educational and learning purposes.
