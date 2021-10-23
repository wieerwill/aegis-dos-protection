#include "DebugHelper.hpp"

void DebugHelper::hex_dump_human_readable (const char *desc, const void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != nullptr)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf (" || %s\n", buff);

            // Output the offset.
            printf ("  %04x ||", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf (" || %s\n", buff);
}

void DebugHelper::hex_dump_raw(const void *addr, int len) {
    unsigned char* pc = (unsigned char*) addr;
    unsigned char res[2 * len + 1];
    res[2 * len] = '\0';

    for(unsigned int i = 0; i < len; i++) {
        printf((char *)res + i * 2, "%02X", pc[i]);
    }
    printf ("%s\n", res);
}
