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

// void Pixel::toArr(byte arr[4]) const {
//   arr[0] = this->r;
//   arr[1] = this->g;
//   arr[2] = this->b;
//   arr[3] = this->a;
// }

// uint32_t Pixel::toRGBA() const {
//   uint32_t res = 0;
//   res ^= this->r;
//   res ^= this->g << 8;
//   res ^= this->b << 16;
//   res ^= this->a << 24;
//   return res;
// }
// uint32_t Pixel::toBGRA() const {
//   uint32_t res = 0;
//   res ^= this->b;
//   res ^= this->g << 8;
//   res ^= this->r << 16;
//   res ^= this->a << 24;
//   return res;
// }

namespace pixel {

byte alpha(Pixel p) {
  return (p & (255 << 24)) >> 24;
}

Pixel over(Pixel front, Pixel back) {
  // TODO
  byte front_arr[4], back_arr[4];
  // undefined behaviour
  *(Pixel*)front_arr = front;
  *(Pixel*)back_arr = back;
  float frontaf = front_arr[3]/255.;
  // TODO pre-multiplied alpha??
  for (int i = 0; i < 4; ++i) {
    front_arr[i] *= frontaf;
    back_arr[i] *= (1-frontaf);
    front_arr[i] += back_arr[i];
  }

  // // undefined behaviour
  return *(Pixel*)front_arr;
}

}

Pixel Sprite::operator()(unsigned h, unsigned w) const {
  size_t pos = h * width + w;
  return pixels[pos];
}

Pixel& Sprite::operator()(unsigned h, unsigned w) {
  size_t pos = h * width + w;
  return pixels[pos];
}


#ifdef DEBUG
uint32_t green = 0 ^ (255 << 24) ^ (255 << 8);
#endif

void SpriteObject::draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
  Box2d sprite_box0{
    .lt = Point2d{.y = 0, .x=0,},
    .rb = Point2d{.y = (int)sprite->getHeight(), .x = (int)sprite->getWidth(),}
  };

  Rectangle rect(sprite_box0);
  TransformChain chain;
  chain.append(std::make_unique<Translation>(rect.translate_(-0.5*sprite_box0.rb)));
  chain.append(std::make_unique<Rotation>(rect.rotate_(rot)));
  chain.append(std::make_unique<Translation>(rect.translate_(pos)));

  Box2d bounding_box = boundingBox2d(rect);
  Box2d screen_box = Box2d{
    .lt = Point2d{.y = 0, .x = 0},
    .rb = Point2d{.y = (int)screen_h, .x = (int)screen_w},
  };
  bounding_box.intersect_(screen_box);
  
  for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      Point2d point = Point2d{.y=i, .x=j};
      if (rect.contains(point)) {
        point = chain.backward(point);

        // finally
        // TODO actual color addition??
        // TODO antialiasing??
        buffer[i*screen_w + j] = pixel::over((*sprite)(point.y, point.x), buffer[i*screen_w + j]);
        // buffer[i*screen_w + j] = (*sprite)(point.y, point.x).toBGRA();
        // buffer[100 + 100*screen_w + i*screen_w + j] = sprite(i, j).toBGRA();
      }
    }
  }

#ifdef DEBUG
  if (bounding_box.lt.x < (int)screen_w) {
    for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
      buffer[i*screen_w + bounding_box.lt.x] = green;
    }
  }
  if (bounding_box.rb.x > 0) {
    for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
      buffer[i*screen_w + bounding_box.rb.x-1] = green;
    }
  }
  if (bounding_box.lt.y < (int)screen_h) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      buffer[bounding_box.lt.y*screen_w + j] = green;
    }
  }
  if (bounding_box.rb.y > 0) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      buffer[(bounding_box.rb.y-1)*screen_w + j] = green;
    }
  }
#endif
}

void SpriteObject::act(float dt) {
  PhysicalObject::act(dt);
  pos.y = y_frac;
  pos.x = x_frac;
}
