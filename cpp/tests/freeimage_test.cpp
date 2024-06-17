// g++ -o build/freeimage_test freeimage_test.cpp -lfreeimage


#include <FreeImage.h>
#include <iostream>

void print_supported_formats() {
    std::cout << "Supported FreeImage formats:" << std::endl;
    for (int i = 0; i < FreeImage_GetFIFCount(); i++) {
        FREE_IMAGE_FORMAT fif = static_cast<FREE_IMAGE_FORMAT>(i);
        std::cout << "  " << FreeImage_GetFormatFromFIF(fif) << ": " << FreeImage_GetFIFExtensionList(fif) << std::endl;
    }
}

bool load_image(const std::string& image_path) {
    FreeImage_Initialise();

    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(image_path.c_str(), 0);
    if (format == FIF_UNKNOWN) {
        std::cerr << "Unknown image format: " << image_path << std::endl;
        format = FreeImage_GetFIFFromFilename(image_path.c_str());
        if (format == FIF_UNKNOWN) {
            std::cerr << "Cannot guess image format from extension: " << image_path << std::endl;
            FreeImage_DeInitialise();
            return false;
        }
    }

    FIBITMAP* image = FreeImage_Load(format, image_path.c_str());
    if (!image) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        FreeImage_DeInitialise();
        return false;
    }

    int width = FreeImage_GetWidth(image);
    int height = FreeImage_GetHeight(image);
    BYTE* data = FreeImage_GetBits(image);

    if (!data) {
        std::cerr << "Failed to get image data: " << image_path << std::endl;
        FreeImage_Unload(image);
        FreeImage_DeInitialise();
        return false;
    }

    std::cout << "Loaded image: " << image_path << " with size " << width << "x" << height << std::endl;

    FreeImage_Unload(image);
    FreeImage_DeInitialise();
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    std::string image_path = argv[1];
    print_supported_formats();
    if (!load_image(image_path)) {
        return -1;
    }

    return 0;
}
