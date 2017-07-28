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

// Runs the test program. Throws various exceptions to signal failures
void run();
// Loads up all the test image, geometrizes it for the given shape tpyes using random settings, and saves the results
void loadGeometrizeAndSaveForTypes(const std::string& filepath, const geometrize::ShapeTypes types);
// Loads up two test images, merges them randomly, geometrizes it using random settings, and saves the results
void mergeGeometrizeAndSave(const std::string& firstFilepath, const std::string& secondFilepath, std::size_t id);

// Geometrizes a bitmap using the given number of steps and shape types, returns the resulting geometrized bitmap
geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps, const geometrize::ShapeTypes shapeTypesOverride);
// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::string& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();
// Helper function to write a PNG file
bool writeImage(const geometrize::Bitmap& bitmap, const std::string& filePath);
// Gets the file paths for all files in the given directory
std::vector<std::string> filepathsForDirectory(const std::string& directory);
// Removes all characters after the last "." in the given path, returning the string without the extension
std::string removeExtension(const std::string& path);
// Searches for the search string in the given string, replacing any instances with the replace string
std::string replaceString(std::string& subject, const std::string& search, const std::string& replace);
// Gets the names of the given shape types
std::string getNamesForShapeTypes(geometrize::ShapeTypes types);

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
    const std::vector<std::string> filepaths{filepathsForDirectory(inputDataDirectory)};

    for(const std::string& filepath : filepaths) {
        for(geometrize::ShapeTypes shape : geometrize::allShapes) {
            loadGeometrizeAndSaveForTypes(filepath, shape);
        }
        loadGeometrizeAndSaveForTypes(filepath, 0); // Random shapes per-step
    }

    const std::size_t mergeCount{100};
    std::random_device rd;
    for(std::size_t i = 0; i < mergeCount; i++) {
        std::uniform_int_distribution<std::size_t> dist{0, filepaths.size() - 1};
        mergeGeometrizeAndSave(filepaths[dist(rd)], filepaths[dist(rd)], i);
    }
}

void loadGeometrizeAndSaveForTypes(const std::string& filepath, const geometrize::ShapeTypes types)
{
    const geometrize::Bitmap bitmap{loadBitmap(filepath)};

    const std::size_t totalSteps = []() {
        std::random_device rd;
        std::uniform_int_distribution<std::size_t> dist{100, 300};
        return static_cast<std::size_t>(dist(rd));
    }();

    std::cout << "Geometrizing file: " << filepath << "\n"
              << "Step count: " << totalSteps << "\n";

    const geometrize::Bitmap result{geometrizeImage(bitmap, totalSteps, types)};

    std::string trimmedPath{removeExtension(filepath)};
    trimmedPath.append("_result" + getNamesForShapeTypes(types) + ".png");
    const std::string destinationPath{replaceString(trimmedPath, "input_data", "output_data")};
    if(!writeImage(result, destinationPath)) {
        throw std::runtime_error("Failed to write image to: " + destinationPath);
    }
}

void mergeGeometrizeAndSave(const std::string& firstFilepath, const std::string& secondFilepath, const std::size_t id)
{
    const geometrize::Bitmap firstBitmap{loadBitmap(firstFilepath)};
    const geometrize::Bitmap secondBitmap{loadBitmap(secondFilepath)};

    const geometrize::Bitmap mergedBitmap = [](const geometrize::Bitmap& first, const geometrize::Bitmap& second) {
        const geometrize::rgba black{0, 0, 0, 255};
        geometrize::Bitmap bitmap(std::min(first.getWidth(), second.getWidth()), std::min(first.getHeight(), second.getHeight()), black);

        for(std::uint32_t x = 0; x < bitmap.getWidth(); x++) {
            for(std::uint32_t y = 0; y < bitmap.getHeight(); y++) {
                const geometrize::rgba a{first.getPixel(x, y)};
                const geometrize::rgba b{second.getPixel(x, y)};

                // NOTE could permute these in interesting ways
                // NOTE could make more test cases
                geometrize::rgba result;
                result.r = a.r * b.r;
                result.g = a.g * b.g;
                result.b = a.b * b.b;
                result.a = a.a * b.a;

                bitmap.setPixel(x, y, result);
            }
        }

        return bitmap;
    }(firstBitmap, secondBitmap);

    const std::size_t totalSteps = []() {
        std::random_device rd;
        std::uniform_int_distribution<std::size_t> dist{100, 300};
        return static_cast<std::size_t>(dist(rd));
    }();

    std::cout << "Geometrizing merged files: " << firstFilepath << " and " << secondFilepath << "\n"
              << "Step count: " << totalSteps << "\n";

    const geometrize::Bitmap result{geometrizeImage(mergedBitmap, totalSteps, 0)};

    std::string trimmedPath{removeExtension(firstFilepath)};
    trimmedPath.append("_merged_result_" + std::to_string(id) + ".png");
    const std::string destinationPath{replaceString(trimmedPath, "input_data", "output_data")};
    if(!writeImage(result, destinationPath)) {
        throw std::runtime_error("Failed to write image to: " + destinationPath);
    }
}

geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps, const geometrize::ShapeTypes shapeTypesOverride)
{
    geometrize::ImageRunner runner{bitmap};
    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        geometrize::ImageRunnerOptions options{generateRandomOptions()};

        if(shapeTypesOverride != 0) {
            options.shapeTypes = shapeTypesOverride;
        }

        std::cout << "Options are: Alpha: " << static_cast<std::int32_t>(options.alpha) << ", "
                  << "Shape types: " << options.shapeTypes << ", "
                  << "Max mutations: " << options.maxShapeMutations << ", "
                  << "Shape count: " << options.shapeCount << ", "
                  << "Random seed: " << options.seed << ", "
                  << "Max threads: " << options.maxThreads << "\n";

        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};

        for(std::size_t i = 0; i < shapes.size(); i++) {
            const float score{shapes[i].score};
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << score << "\n";
            if(score < 0.0f || score > 1.0f) {
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
    if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0 || bitmap.getDataRef().size() == 0) {
        throw std::runtime_error("Loaded empty bitmap");
    }

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

std::vector<std::string> filepathsForDirectory(const std::string& directory)
{
    std::vector<std::string> filepaths;
    for(auto& p : std::experimental::filesystem::directory_iterator(directory)) {
        filepaths.push_back(std::experimental::filesystem::canonical(p.path()).string());
    }
    return filepaths;
}

std::string removeExtension(const std::string& path)
{
    std::size_t lastdot = path.find_last_of(".");
    if (lastdot == std::string::npos) {
        return path;
    }
    return path.substr(0, lastdot);
}

std::string replaceString(std::string& subject, const std::string& search, const std::string& replace)
{
    std::size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

std::string getNamesForShapeTypes(const geometrize::ShapeTypes types)
{
    std::string result{""};
    for(const auto& t : geometrize::shapeTypeNames) {
        if((t.first & types) == t.first) {
            result += "_" + t.second;
        }
    }
    return result;
}

}
