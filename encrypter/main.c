/*
*****************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
*****************************************************************************
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hex_parser.h"
#include "aes.h"


//#define SHOW_SECRET
//#define SHOW_ORIGIN
//#define SHOW_ENCRYPTED


#define OUT_FILE_EXT    ".enc"

extern const uint8_t _binary____keys_iv_bin_start[];
extern const uint8_t _binary____keys_iv_bin_end[];
extern const uint8_t _binary____keys_key_bin_start[];
extern const uint8_t _binary____keys_key_bin_end[];


static struct AES_ctx ctx;


bool hex_parser_new_data(uint32_t addr, uint8_t data[], uint16_t len) {
    if(data) {
#ifdef SHOW_ORIGIN
        printf("%08X:", addr);
        for(uint16_t i = 0; i < len; i++) {
            printf(" %02X", data[i]);
        }
        printf("\n");
#endif
        AES_CTR_xcrypt_buffer(&ctx, data, len);
#ifdef SHOW_ENCRYPTED
        printf("%08X:", addr);
        for(uint16_t i = 0; i < len; i++) {
            printf(" %02X", data[i]);
        }
        printf("\n");
#endif
    }
    return true;
}


int main(int argc, char* argv[]) {
    printf("HEX file AES encrypter\n");
#ifdef SHOW_SECRET
    printf("IV: ");
    for(const uint8_t * bt = _binary____keys_iv_bin_start; bt != _binary____keys_iv_bin_end; ++bt) {
        printf(" %02X", *bt);
    }
    printf("\n");

    printf("KEY:");
    for(const uint8_t * bt = _binary____keys_key_bin_start; bt != _binary____keys_key_bin_end; ++bt) {
        printf(" %02X", *bt);
    }
    printf("\n");
#endif

    if(argc != 2) {
        printf("Usage %s <hex_filename>\n", argv[0]);
        return 1;
    }

    FILE * f_input;
    f_input = fopen(argv[1], "r");
    if(!f_input) {
        printf("Hex file %s not found\n", argv[1]);
        return 1;
    }

    char * out_name = malloc(strlen(OUT_FILE_EXT) + strlen(argv[1]) + 1);
    if(!out_name) {
        fclose(f_input);
        printf("Memory allocation error\n");
        return 1;
    }
    strcpy(out_name, argv[1]);
    strcpy(&out_name[strlen(out_name)], OUT_FILE_EXT);
    FILE * f_output;
    f_output = fopen(out_name, "w");
    free(out_name);
    if(!f_output) {
        fclose(f_input);
        printf("Hex file %s not found\n", argv[1]);
        return 1;
    }

    hex_parser_set_callback(hex_parser_new_data);
    AES_init_ctx_iv(&ctx, _binary____keys_key_bin_start, _binary____keys_iv_bin_start);

    char * line = NULL;
    size_t len = 0;
    while(getline(&line, &len, f_input) != -1) {
        if(strlen(line) >= 2) {
            line[strlen(line) - 2] = '\0';
            if(!hex_parser_feed(line, true)) {
                printf("Unparsed HEX line: %s\n", line);
            } else {
                fprintf(f_output, "%s\r\n", line);
            }
        }
    }
    if (line) {
        free(line);
    }

    fclose(f_output);
    fclose(f_input);

    printf("All done\n");

    return 0;
}
