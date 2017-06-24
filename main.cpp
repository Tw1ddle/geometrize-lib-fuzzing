#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <utility>

#include "lib/libbmpread/bmpread.h"

#include "geometrize/shaperesult.h"
#include "geometrize/bitmap/bitmap.h"
#include "geometrize/bitmap/rgba.h"
#include "geometrize/shape/shape.h"
#include "geometrize/shape/shapetypes.h"
#include "geometrize/runner/imagerunner.h"
#include "geometrize/runner/imagerunneroptions.h"

namespace {

// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::string& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();

}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        assert(0 && "No input bitmap specified");
        return 1;
    }
    const geometrize::Bitmap bitmap{loadBitmap(argv[1])};
    const geometrize::ImageRunnerOptions options{generateRandomOptions()};

    geometrize::ImageRunner runner{bitmap};
    if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0 || bitmap.getDataRef().size() == 0) {
        assert(0 && "Loaded empty bitmap");
        return 2;
    }

    const std::size_t totalSteps = []() {
        std::random_device rd;
        std::uniform_int_distribution<std::size_t> dist{0, 512};
        return static_cast<std::size_t>(dist(rd));
    }();

    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};
        for(std::size_t i = 0; i < shapes.size(); i++) {
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << shapes[i].score << "\n";
        }
    }

    return 0;
}

namespace {

geometrize::Bitmap loadBitmap(const std::string& filePath)
{
    bmpread_t bitmap;
    if(!bmpread(filePath.c_str(), 0, &bitmap)) {
        return geometrize::Bitmap(0, 0, geometrize::rgba{0, 0, 0, 0}); // Failed to read, return empty
    }
    std::vector<std::uint8_t> data(bitmap.rgb_data, bitmap.rgb_data + (bitmap.width * bitmap.height * 3));
    return geometrize::Bitmap(bitmap.width, bitmap.height, data);
}

geometrize::ImageRunnerOptions generateRandomOptions()
{
    geometrize::ImageRunnerOptions options;

    std::random_device rd;

    options.shapeTypes = [&rd]() {
        return geometrize::ShapeTypes::ELLIPSE;
    }();
    options.alpha = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{0, 255};
        return static_cast<std::uint8_t>(dist(rd) & 0xFF);
    }();
    options.shapeCount = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{1, 200};
        return dist(rd);
    }();
    options.maxShapeMutations = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{1, 200};
        return dist(rd);
    }();
    options.seed = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{0, UINT_MAX};
        return dist(rd);
    }();
    options.maxThreads = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{0, 16};
        return static_cast<std::uint8_t>(dist(rd) & 0x10);
    }();

    return options;
}

}
