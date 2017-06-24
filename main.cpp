#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <utility>

#include <experimental/filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb/stb_image_write.h"

#include "geometrize/shaperesult.h"
#include "geometrize/bitmap/bitmap.h"
#include "geometrize/bitmap/rgba.h"
#include "geometrize/shape/shape.h"
#include "geometrize/shape/shapetypes.h"
#include "geometrize/runner/imagerunner.h"
#include "geometrize/runner/imagerunneroptions.h"

namespace {

// Geometrizes the bitmap using the given options and steps
void run(const geometrize::Bitmap bitmap, const geometrize::ImageRunnerOptions options, const std::size_t totalSteps);
// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::string& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();

}

int main(int /*argc*/, char /**argv[]*/)
{
    for(auto& p : std::experimental::filesystem::directory_iterator("../geometrize-lib-fuzzing/input_data")) {
        const geometrize::Bitmap bitmap{loadBitmap(std::experimental::filesystem::canonical(p.path()).string())};
        if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0 || bitmap.getDataRef().size() == 0) {
            assert(0 && "Loaded empty bitmap");
            return 2;
        }

        const geometrize::ImageRunnerOptions options{generateRandomOptions()};
        const std::size_t totalSteps = []() {
            std::random_device rd;
            std::uniform_int_distribution<std::size_t> dist{0, 300};
            return static_cast<std::size_t>(dist(rd));
        }();

        run(bitmap, options, totalSteps);
    }

    return 0;
}

namespace {

void run(const geometrize::Bitmap bitmap, const geometrize::ImageRunnerOptions options, const std::size_t totalSteps)
{
    geometrize::ImageRunner runner{bitmap};
    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};
        for(std::size_t i = 0; i < shapes.size(); i++) {
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << shapes[i].score << "\n";
        }
    }
}

geometrize::Bitmap loadBitmap(const std::string& filePath) // Helper function to read an image file to RGBA8888 pixel data
{
    const char* path{filePath.c_str()};
    std::int32_t w = 0;
    std::int32_t h = 0;
    std::int32_t n = 0;
    std::uint8_t* dataPtr{stbi_load(path, &w, &h, &n, 4)};
    if(dataPtr == nullptr) {
        return geometrize::Bitmap(0, 0, geometrize::rgba{0, 0, 0, 0});
    }
    const std::vector<std::uint8_t> data{dataPtr, dataPtr + (w * h * 4)};
    delete dataPtr;

    const geometrize::Bitmap bitmap(w, h, data);
    return bitmap;
}

geometrize::ImageRunnerOptions generateRandomOptions()
{
    geometrize::ImageRunnerOptions options;

    std::random_device rd;

    options.shapeTypes = [&rd]() {
        std::uniform_int_distribution<std::size_t> dist{0, geometrize::allShapes.size() - 1};
        return geometrize::allShapes[dist(rd)];
    }();
    options.alpha = [&rd]() {
        std::uniform_int_distribution<std::uint32_t> dist{0, 255};
        return static_cast<std::uint8_t>(dist(rd));
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
        return static_cast<std::uint8_t>(dist(rd));
    }();

    return options;
}

}
