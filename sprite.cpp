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

  // undefined behaviour
  return *(Pixel*)front_arr;
}

}

// Pixel Sprite::operator()(unsigned h, unsigned w) const {
//   size_t pos = h * width + w;
//   return pixels[pos];
// }

Pixel& Sprite::operator()(unsigned h, unsigned w) {
  size_t pos = h * width + w;
  return pixels[pos];
}

Pixel weightedMerge(
  Pixel l, Pixel r,
  float ilw /* inverse left weight */, float irw /* inverse right weight */,
  float eps = 1e-02
)
{
  byte arr[4];
  if (ilw < eps) {
    return l;
  }
  if (irw < eps) {
    return r;
  }
  float lw = irw;
  float rw = ilw;
  float n = lw + rw; // normalizer
  for (int i = 0; i < 4; ++i) {
    int shift = 8*i;
    int lval = ((l & (255 << shift)) >> shift);
    int rval = ((r & (255 << shift)) >> shift);
    int val = (lw * lval + rw * rval) / n;
    if (val > 255) { val = 255; }
    if (val < 0) { val = 0; }
    arr[i] = val;
  }
  // undefined behaviour
  return *(Pixel*)arr;
}
Pixel Sprite::operator()(float h, float w) const {
  if (h < 0) h = 0;
  if (h >= height) h = height-1;
  if (w < 0) w = 0;
  if (w >= width) w = width-1;
  int wf = floor(w);
  int wc = ceil(w);
  int hf = floor(h);
  int hc = ceil(h);
  float ilw = w - wf; // inverse left weight
  float irw = wc - w; // inverse right weight
  float itw = h - hf; // inverse top weight
  float ibw = hc - h; // inverse bottom weight
  Pixel top = weightedMerge(
      pixels[hf * width + wf], pixels[hf * width + wc],
      ilw, irw
    );
  Pixel bottom = weightedMerge(
      pixels[hc * width + wf], pixels[hc * width + wc],
      ilw, irw
    );
  return weightedMerge(top, bottom, itw, ibw);
}

// Pixel& Sprite::operator()(float h, float w) {
//   if (h < 0) h = 0;
//   if (h > height) h = height-1;
//   if (w < 0) w = 0;
//   if (w > width) w = width-1;
//   int wf = floor(w);
//   int wc = ceil(w);
//   int hf = floor(h);
//   int hc = ceil(h);
//   float ilw = w - wf; // inverse left weight
//   float irw = wc - w; // inverse right weight
//   float itw = h - hf; // inverse top weight
//   float ibw = hc - h; // inverse bottom weight
//   Pixel top = weightedMerge(
//       pixels[hf * width + wf], pixels[hf * width + wc],
//       ilw, irw
//     );
//   Pixel bottom = weightedMerge(
//       pixels[(hf+1) * width + wf], pixels[(hf+1) * width + wc],
//       ilw, irw
//     );
//   // size_t pos = h * width + w;
//   // return pixels[pos];
//   return weightedMerge(top, bottom, itw, ibw);
// }

#ifdef DEBUG
uint32_t green = 0 ^ (255 << 24) ^ (255 << 8);
#endif

void SpriteObject::draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
  Box2d sprite_box0{
    .lt = Point2d{.y = 0, .x=0,},
    .rb = Point2d{.y = (int)sprite->getHeight(), .x = (int)sprite->getWidth(),}
  };

  Rectangle rect(sprite_box0);
  // TransformChain chain;
  // chain.append(std::make_unique<Translation>(rect.translate_(-0.5*sprite_box0.rb)));
  // chain.append(std::make_unique<Rotation>(rect.rotate_(rot)));
  // chain.append(std::make_unique<Translation>(rect.translate_(pos)));
  TransformChainF chain;
  chain.append(std::make_unique<TranslationF>(rect.translatef_(-0.5*sprite_box0.rb)));
  chain.append(std::make_unique<RotationF>(rect.rotatef_(rot)));
  chain.append(std::make_unique<TranslationF>(rect.translatef_(pos)));

  Box2d bounding_box = boundingBox2d(rect);
  Box2d screen_box = Box2d{
    .lt = Point2d{.y = 0, .x = 0},
    .rb = Point2d{.y = (int)screen_h, .x = (int)screen_w},
  };
  bounding_box.intersect_(screen_box);
  
  for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      Point2dF point = Point2dF{.y=(float)i, .x=(float)j};
      Point2d pointi = Point2d{.y=i, .x=j};
      if (rect.contains(pointi)) {
        point = chain.backward(point);

        // finally
        // TODO actual color addition??
        // TODO antialiasing??
        // somehow fails
        buffer[i*screen_w + j] = pixel::over(
            static_cast<const Sprite&>(*sprite)(point.y, point.x),
            buffer[i*screen_w + j]
          );
        // buffer[i*screen_w + j] = pixel::over(
        //     (*sprite)((unsigned)point.y, (unsigned)point.x),
        //     buffer[i*screen_w + j]
        //   );
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
