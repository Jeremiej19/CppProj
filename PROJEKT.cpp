#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>
#include <algorithm>
#include <windows.h>
#include <Windows.h>
#include <bitset>
#include <filesystem>

using namespace std;
using namespace Magick;

#define SECRET_LENGHT 2
#define COLOR_BITS 8
#define SECRET_MASK ((1 << SECRET_LENGHT) -1)

void hide(string file1, string file2, string outFileName = "out.png") {

    string srcdir("");
    //cout << srcdir + "flip_out.png" << endl;

    // Read images into STL list
    list<Magick::Image> readimageList;
    readImages(&readimageList, srcdir + file1);
    Magick::Image image = *readimageList.begin();
    readimageList.clear();

    readImages(&readimageList, srcdir + file2);
    Magick::Image image2 = *readimageList.begin();
    readimageList.clear();


    auto a = image.size();
    a.aspect(true);
    image2.resize(a);

    //cout << image.columns() << " " << image.rows() << " " << image.channels() << endl;
    //cout << image2.columns() << " " << image2.rows() << " " << image2.channels() << endl;
    size_t cols = image.columns();
    size_t rows = image.rows();

    Pixels view(image);
    MagickCore::Quantum* pixels = view.get(0, 0, cols, rows);
    image.modifyImage();
    Pixels view2(image2);
    MagickCore::Quantum* pixelsSecret = view2.get(0, 0, cols, rows);

    int baseChannels = image.channels();
    int secretChannels = image2.channels();

    MagickCore::Quantum* secretPointer = pixelsSecret;
    int i = 0;
    for (ssize_t row = 0; row < rows * baseChannels; row += baseChannels)
        for (ssize_t column = 0; column < cols * baseChannels; column += baseChannels)
        {
            i += 3;
            //QuantumRange; //MagickCore::SetQuantumFormat(MagickCore::);
            auto red = MagickCore::ScaleQuantumToChar(pixels[cols * row + column + 0]);
            auto green = MagickCore::ScaleQuantumToChar(pixels[cols * row + column + 1]);
            auto blue = MagickCore::ScaleQuantumToChar(pixels[cols * row + column + 2]);

            red = (unsigned short)red >> SECRET_LENGHT;
            green = (unsigned short)green >> SECRET_LENGHT;
            blue = (unsigned short)blue >> SECRET_LENGHT;

            red = (unsigned short)red << SECRET_LENGHT;
            green = (unsigned short)green << SECRET_LENGHT;
            blue = (unsigned short)blue << SECRET_LENGHT;

            unsigned short redSecret = MagickCore::ScaleQuantumToChar(*secretPointer);
            ++secretPointer;
            unsigned short greenSecret = MagickCore::ScaleQuantumToChar(*secretPointer);
            ++secretPointer;
            unsigned short blueSecret = MagickCore::ScaleQuantumToChar(*secretPointer);
            if (secretChannels == 4) {
                secretPointer += 2;
            }
            else {
                ++secretPointer;
            }
            redSecret = (unsigned short)redSecret >> (COLOR_BITS - SECRET_LENGHT);
            greenSecret = (unsigned short)greenSecret >> (COLOR_BITS - SECRET_LENGHT);
            blueSecret = (unsigned short)blueSecret >> (COLOR_BITS - SECRET_LENGHT);
            red += redSecret;
            green += greenSecret;
            blue += blueSecret;

            pixels[cols * row + column + 0] = (257.0) * red;
            pixels[cols * row + column + 1] = (257.0) * green;
            pixels[cols * row + column + 2] = (257.0) * blue;
        }

    image.syncPixels();

    list<Magick::Image> imageList;
    imageList.push_back(image);
    writeImages(imageList.begin(), imageList.end(), outFileName);
    imageList.clear();
}

void show(string file , string outFileName = "secret.png") {
    string srcdir("");

    // Read images into STL list
    list<Magick::Image> readimageList;
    readImages(&readimageList, srcdir + file);
    Magick::Image secretImage = *readimageList.begin();
    readimageList.clear();

    size_t cols = secretImage.columns();
    size_t rows = secretImage.rows();

    Pixels viewSecret(secretImage);
    MagickCore::Quantum* pixelsSecret2 = viewSecret.get(0, 0, cols, rows);
    secretImage.modifyImage();

    //unsigned char* newPixels = new unsigned char[cols * rows * 4];
    auto newPixels = make_unique<unsigned char[]>(cols * rows * 4);

    //cout << secretImage.columns() << " " << secretImage.rows() << " " << secretImage.channels() << endl;
    int channels = secretImage.channels();

    for (ssize_t row = 0; row < rows * channels; row += channels)
        for (ssize_t column = 0; column < cols * channels; column += channels)
        {
            QuantumRange;
            unsigned char red = MagickCore::ScaleQuantumToChar(pixelsSecret2[cols * row + column + 0]);
            unsigned char green = MagickCore::ScaleQuantumToChar(pixelsSecret2[cols * row + column + 1]);
            unsigned char blue = MagickCore::ScaleQuantumToChar(pixelsSecret2[cols * row + column + 2]);

            red = red & SECRET_MASK;
            green = green & SECRET_MASK;
            blue = blue & SECRET_MASK;


            red = red << (COLOR_BITS - SECRET_LENGHT);
            green = green << (COLOR_BITS - SECRET_LENGHT);
            blue = blue << (COLOR_BITS - SECRET_LENGHT);

            newPixels[cols * row + column + 0] = red;
            newPixels[cols * row + column + 1] = green;
            newPixels[cols * row + column + 2] = blue;
        }
    secretImage.syncPixels();


    readImages(&readimageList, srcdir + file);
    Magick::Image outFile = *readimageList.begin();
    readimageList.clear();

    Pixels outView(outFile);
    MagickCore::Quantum* outPixels = outView.get(0, 0, cols, rows);
    outFile.modifyImage();

    cols = outFile.columns();
    rows = outFile.rows();
    for (ssize_t row = 0; row < rows * channels; row += channels)
        for (ssize_t column = 0; column < cols * channels; column += channels)
        {
            QuantumRange;
            outPixels[cols * row + column + 0] = (257.0) * newPixels[cols * row + column + 0];
            outPixels[cols * row + column + 1] = (257.0) * newPixels[cols * row + column + 1];
            outPixels[cols * row + column + 2] = (257.0) * newPixels[cols * row + column + 2];

        }
    outFile.syncPixels();

    list<Magick::Image> imageList;
    imageList.push_back(outFile);
    writeImages(imageList.begin(), imageList.end(), outFileName);
    imageList.clear();
}

int main(int argc, char** argv)
{
    // Initialize ImageMagick install location for Windows
    InitializeMagick(*argv);

    if (argc > 1 && strncmp(argv[1],"show",2) == 0) {
        if (argc == 4)
            show(argv[2],argv[3]);
        else
            show(argv[2]);
    }
    else if (argc > 1 && strncmp(argv[1], "hide", 2) == 0) {
        string s = string(argv[2]);
        int pos = s.rfind('.', s.length());
        string ext = s.substr(pos + 1, s.length() - pos);
        string defFile = "out." + ext;
        if (argc == 5)
            hide(argv[2], argv[3], argv[4]);
        else
            hide(argv[2], argv[3], defFile);
        
    }
    else {
        cout << "Available Commands:\n" <<
            "- hide : Hides one image into another.\n" <<
            "  Usage: hide <source_image_path> <hidden_image_path> [<output_image_path>]\n" <<
            "  Example: hide image1.jpg image2.png output.jpg\n\n" <<
            "- show : Retrieves the hidden image from an image.\n" <<
            "  Usage: show <image_with_hidden_message_path> [<output_img_path>]\n" <<
            "  Example: show steganography_image.jpg hidden.jpg" << endl;
    }

    return 0;
}