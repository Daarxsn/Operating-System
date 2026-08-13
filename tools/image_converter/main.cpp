#include <iostream>
#include <fstream>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

int main()
{
    const char *input =
        "../../assets/images/xyrisos_logo.png";

    int width;
    int height;
    int channels;

    unsigned char *pixels =
        stbi_load(
            input,
            &width,
            &height,
            &channels,
            4
        );

    if (!pixels)
    {
        std::cout << "Failed to load image\n";
        return 1;
    }

    std::ofstream header(
        "../../kernel/image/logo.h"
    );

    std::ofstream source(
        "../../kernel/image/logo.c"
    );

    /* -------------------------------------------------
       Header File
    ------------------------------------------------- */

    header << R"(#ifndef XYRIS_LOGO_H
#define XYRIS_LOGO_H

#include <stdint.h>
#include "image.h"

extern const Image xyris_logo;

#endif
)";

    /* -------------------------------------------------
       Source File
    ------------------------------------------------- */

    source << "#include \"logo.h\"\n\n";

    source << "static const uint32_t logo_pixels[] =\n{\n";

    for (int i = 0; i < width * height; i++)
    {
        uint8_t r = pixels[i * 4 + 0];
        uint8_t g = pixels[i * 4 + 1];
        uint8_t b = pixels[i * 4 + 2];
        uint8_t a = pixels[i * 4 + 3];

        uint32_t argb =
    ((uint32_t)a << 24) |
    ((uint32_t)r << 16) |
    ((uint32_t)g << 8)  |
    (uint32_t)b;

       source
    << "0x"
    << std::hex
    << std::setw(8)
    << std::setfill('0')
    << std::uppercase
    << argb
    << ",";

        if ((i + 1) % 8 == 0)
            source << "\n";
    }

    source << "\n};\n\n";

    /* -------------------------------------------------
       IMPORTANT
       Switch back to decimal
    ------------------------------------------------- */

    source << std::dec;

    source << "const Image xyris_logo =\n";
    source << "{\n";
    source << "    " << width << ",\n";
    source << "    " << height << ",\n";
    source << "    logo_pixels\n";
    source << "};\n";

    stbi_image_free(pixels);

    std::cout
        << "Generated logo.c and logo.h\n";

    return 0;
}