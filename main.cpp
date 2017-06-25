#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

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

// Geometrizes a bitmap using the given number of steps, returns the resulting geometrized bitmap
geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps);
// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::string& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();
// Helper function to write a PNG file
bool writeImage(const geometrize::Bitmap& bitmap, const std::string& filePath);
// Runs the test program. Throws various exceptions to signal failure.
void run();
}

int main(int /*argc*/, char /**argv[]*/)
{
    run();
    return 0;
}

namespace {

void run()
{
    for(auto& p : std::experimental::filesystem::directory_iterator("../geometrize-lib-fuzzing/input_data")) {
        const std::string filepath{std::experimental::filesystem::canonical(p.path()).string()};
        const geometrize::Bitmap bitmap{loadBitmap(filepath)};
        if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0 || bitmap.getDataRef().size() == 0) {
            assert(0 && "Loaded empty bitmap");
            return;
        }

        const std::size_t totalSteps = []() {
            std::random_device rd;
            std::uniform_int_distribution<std::size_t> dist{100, 300};
            return static_cast<std::size_t>(dist(rd));
        }();

        std::cout << "Geometrizing file: " << filepath << "\n"
                  << "Step count: " << totalSteps << "\n";


        auto removeExtension = [](const std::string& path) -> std::string {
            std::size_t lastdot = path.find_last_of(".");
            if (lastdot == std::string::npos) {
                return path;
            }
            return path.substr(0, lastdot);
        };
        auto replaceString = [](std::string& subject, const std::string& search, const std::string& replace) -> std::string {
            std::size_t pos = 0;
            while((pos = subject.find(search, pos)) != std::string::npos) {
                 subject.replace(pos, search.length(), replace);
                 pos += replace.length();
            }
            return subject;
        };

        const geometrize::Bitmap result{geometrizeImage(bitmap, totalSteps)};
        writeImage(result, replaceString(removeExtension(filepath) + "_result.png", "input_data", "output_data"));
    }
}

geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps)
{
    geometrize::ImageRunner runner{bitmap};
    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        const geometrize::ImageRunnerOptions options{generateRandomOptions()};
        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};
        for(std::size_t i = 0; i < shapes.size(); i++) {
            const float score{shapes[i].score};
            assert(score <= 1.0f);
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << score << "\n";
        }

        std::cout << "Options were: Alpha: " << static_cast<std::int32_t>(options.alpha) << ", "
                  << "Max shapes: " << options.shapeCount << ", "
                  << "Max mutations: " << options.maxShapeMutations << ", "
                  << "Shape count: " << options.shapeCount << ", "
                  << "Random seed: " << options.seed << ", "
                  << "Max threads: " << options.maxThreads << "\n";
    }
    return runner.getCurrent();
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
        std::uniform_int_distribution<std::uint32_t> dist{0, 16}; // Note selecting 0 threads should let the implementation choose
        return static_cast<std::uint8_t>(dist(rd));
    }();

    return options;
}

bool writeImage(const geometrize::Bitmap& bitmap, const std::string& filePath)
{
    const char* path{filePath.c_str()};
    const void* data{bitmap.getDataRef().data()};
    return stbi_write_png(path, bitmap.getWidth(), bitmap.getHeight(), 4, data, bitmap.getWidth() * 4) != 0;
}

}
