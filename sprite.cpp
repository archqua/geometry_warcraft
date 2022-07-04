#include "sprite.h"
// #include <fstream>
// #include <cmath>

// https://stackoverflow.com/questions/15366319/how-to-read-the-binary-file-in-c#15366657
// std::vector<BYTE> readFile(const char* filename)
// {
//     // open the file:
//     std::streampos fileSize;
//     std::ifstream file(filename, std::ios::binary);

//     // get its size:
//     file.seekg(0, std::ios::end);
//     fileSize = file.tellg();
//     file.seekg(0, std::ios::beg);

//     // read the data:
//     std::vector<BYTE> fileData(fileSize);
//     file.read((char*) &fileData[0], fileSize);
//     return fileData;
// }

// static
// template <unsigned bitcount>
// Sprite Sprite::fromBinaryFile(const char *file)
// {
//   std::ifstream fs(file, std::ios::binary);
//   if (fs) {
//     unsigned height = 0, width = 0;
//     byte h[bitcount];
//     byte w[bitcount];
//     fs.read((char*) h, bitcount);
//     fs.read((char*) w, bitcount);
//     for (int i = 0; i < bitcount; ++i) {
//       height += pow(256, i) * h[bitcount-1-i];
//       width += pow(256, i) * w[bitcount-1-i];
//     }
//     Sprite res(height, width);
//     try {
//       for (PixelIterator iter = res.pixelsBegin(); iter != res.pixelsEnd(); ++iter) {
//         fs.read((char*) &(iter->r), 1);
//         fs.read((char*) &(iter->g), 1);
//         fs.read((char*) &(iter->b), 1);
//         fs.read((char*) &(iter->a), 1);
//       }
//     return res;
//     } catch (const std::ios_base::failure&) {
//       throw FRError(file);
//     }
//   } else {
//     throw FOError(file);
//   }
// }

void Pixel::toArr(byte arr[4]) const {
  arr[0] = this->r;
  arr[1] = this->g;
  arr[2] = this->b;
  arr[3] = this->a;
}

uint32_t Pixel::toRGBA() const {
  uint32_t res = 0;
  res ^= this->r;
  res ^= this->g << 8;
  res ^= this->b << 16;
  res ^= this->a << 24;
  return res;
}
uint32_t Pixel::toBGRA() const {
  uint32_t res = 0;
  res ^= this->b;
  res ^= this->g << 8;
  res ^= this->r << 16;
  res ^= this->a << 24;
  return res;
}

Pixel Sprite::operator()(unsigned h, unsigned w) const {
  size_t pos = h * width + w;
  return pixels[pos];
}

Pixel& Sprite::operator()(unsigned h, unsigned w) {
  size_t pos = h * width + w;
  return pixels[pos];
}



