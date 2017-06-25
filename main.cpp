#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
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

const std::string inputDataDirectory{"../geometrize-lib-fuzzing/input_data"};

// Geometrizes a bitmap using the given number of steps, returns the resulting geometrized bitmap
geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps);
// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::string& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();
// Helper function to write a PNG file
bool writeImage(const geometrize::Bitmap& bitmap, const std::string& filePath);
// Runs the test program. Throws various exceptions to signal failures.
void run();
}

int main(int /*argc*/, char** /*argv*/)
{
    try {
        run();
    } catch(const std::exception& e) {
        std::cout << "Encountered fatal exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

namespace {

void run()
{
    for(auto& p : std::experimental::filesystem::directory_iterator(inputDataDirectory)) {
        const std::string filepath{std::experimental::filesystem::canonical(p.path()).string()};
        const geometrize::Bitmap bitmap{loadBitmap(filepath)};
        if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0 || bitmap.getDataRef().size() == 0) {
            throw std::runtime_error("Loaded empty bitmap");
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

        std::string trimmedPath{removeExtension(filepath)};
        trimmedPath.append("_result.png");
        const std::string destinationPath{replaceString(trimmedPath, "input_data", "output_data")};
        if(!writeImage(result, destinationPath)) {
            throw std::runtime_error("Failed to write image to: " + destinationPath);
        }
    }
}

geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps)
{
    geometrize::ImageRunner runner{bitmap};
    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        const geometrize::ImageRunnerOptions options{generateRandomOptions()};
        std::cout << "Options are: Alpha: " << static_cast<std::int32_t>(options.alpha) << ", "
                  << "Max shapes: " << options.shapeCount << ", "
                  << "Max mutations: " << options.maxShapeMutations << ", "
                  << "Shape count: " << options.shapeCount << ", "
                  << "Random seed: " << options.seed << ", "
                  << "Max threads: " << options.maxThreads << "\n";

        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};

        for(std::size_t i = 0; i < shapes.size(); i++) {
            const float score{shapes[i].score};
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << score << "\n";
            if(score > 1.0f) {
                throw std::runtime_error("Shape has invalid score: " + std::to_string(score));
            }
        }
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
        throw std::runtime_error("Failed to load image: " + filePath);
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
