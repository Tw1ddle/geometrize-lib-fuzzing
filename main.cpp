#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <QDirIterator>
#include <QImage>

#include "geometrize/shaperesult.h"
#include "geometrize/bitmap/bitmap.h"
#include "geometrize/bitmap/rgba.h"
#include "geometrize/shape/shape.h"
#include "geometrize/shape/shapetypes.h"
#include "geometrize/runner/imagerunner.h"
#include "geometrize/runner/imagerunneroptions.h"

namespace {

const std::filesystem::path inputDataDirectory{"../geometrize-lib-fuzzing/input_data"};
const std::filesystem::path outputDataDirectory{"../geometrize-lib-fuzzing/output_data"};

// Runs the test program. Throws various exceptions to signal failures
void run();
// Creates the input/output directories for the tests
void createInputOutputDirectories();
// Loads a test image, geometrizes it for the given shape types using otherwise random settings, and saves the result
void loadGeometrizeAndSaveForTypes(const std::filesystem::path& filepath, const geometrize::ShapeTypes types);
// Loads two test images, merges the images together, and geometrizes the image using random settings, and saves the result
void mergeGeometrizeAndSave(const std::filesystem::path& firstFilepath, const std::filesystem::path& secondFilepath, std::size_t id);

// Geometrizes a bitmap using the given number of steps and shape types, returns the resulting geometrized bitmap
geometrize::Bitmap geometrizeImage(const geometrize::Bitmap bitmap, const std::size_t totalSteps, const geometrize::ShapeTypes shapeTypesOverride);
// Load a Windows bitmap (bmp) from a file
geometrize::Bitmap loadBitmap(const std::filesystem::path& filePath);
// Generate randomized image runner options
geometrize::ImageRunnerOptions generateRandomOptions();
// Helper function to write a PNG file
bool writeImage(const geometrize::Bitmap& bitmap, const std::filesystem::path& filePath);
// Gets the file paths for all files in the given directory
std::vector<std::filesystem::path> filepathsForDirectory(const std::filesystem::path& directory);
// Removes all characters after the last "." in the given path, returning the string without the extension
std::filesystem::path removeExtension(const std::filesystem::path& path);
// Searches for the search string in the given string, replacing any instances with the replace string
std::string replaceString(std::string subject, const std::string& search, const std::string& replace);
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
    } catch(...) {
        std::cout << "Encountered unknown/unhandled exception\n";
        return 2;
    }

    return 0;
}

namespace {

void run()
{
    createInputOutputDirectories();

    const std::vector<std::filesystem::path> filepaths{filepathsForDirectory(inputDataDirectory)};

    for(const std::filesystem::path& filepath : filepaths) {
        for(geometrize::ShapeTypes shape : geometrize::allShapes) {
            loadGeometrizeAndSaveForTypes(filepath, shape);
        }
        loadGeometrizeAndSaveForTypes(filepath, static_cast<geometrize::ShapeTypes>(0)); // Use random shape types on each step
    }

    const std::size_t mergeTests{100};
    std::random_device rd;
    for(std::size_t i = 0; i < mergeTests; i++) {
        std::uniform_int_distribution<std::size_t> dist{0, filepaths.size() - 1};
        mergeGeometrizeAndSave(filepaths[dist(rd)], filepaths[dist(rd)], i);
    }
}

void createInputOutputDirectories()
{
    std::filesystem::create_directories(std::filesystem::absolute(inputDataDirectory));
    std::filesystem::create_directories(std::filesystem::absolute(outputDataDirectory));
}

void loadGeometrizeAndSaveForTypes(const std::filesystem::path& filepath, const geometrize::ShapeTypes types)
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

    std::string trimmedPath(removeExtension(filepath).string());
    trimmedPath.append("_result" + getNamesForShapeTypes(types) + ".png");
    const std::string destinationPath{replaceString(trimmedPath, "input_data", "output_data")};
    if(!writeImage(result, destinationPath)) {
        throw std::runtime_error("Failed to write image to: " + destinationPath);
    }
}

void mergeGeometrizeAndSave(const std::filesystem::path& firstFilepath, const std::filesystem::path& secondFilepath, const std::size_t id)
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

    std::cout << "Geometrizing merged files: " << firstFilepath.string() << " and " << secondFilepath << "\n"
              << "Step count: " << totalSteps << "\n";

    const geometrize::Bitmap result{geometrizeImage(mergedBitmap, totalSteps, static_cast<geometrize::ShapeTypes>(0))};

    std::filesystem::path trimmedPath{removeExtension(firstFilepath)};
    trimmedPath.append("_merged_result_" + std::to_string(id) + ".png");
    const std::filesystem::path destinationPath{std::filesystem::path(replaceString(trimmedPath.string(), "input_data", "output_data"))};
    if(!writeImage(result, destinationPath)) {
        throw std::runtime_error("Failed to write image to: " + destinationPath.string());
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
            const double score{shapes[i].score};
            std::cout << "Added shape " << steps + i << ". Type: " << shapes[i].shape->getType() << ". Score: " << score << "\n";
            if(score < 0.0f || score > 1.0f) {
                throw std::runtime_error("Shape has invalid score: " + std::to_string(score));
            }
        }
    }
    return runner.getCurrent();
}

geometrize::Bitmap loadBitmap(const std::filesystem::path& filePath) // Helper function to read an image file to RGBA8888 pixel data
{
    QImage image(QString::fromStdString(filePath.string()));
    if(image.isNull()) {
        throw std::runtime_error("Failed to load image: " + filePath.string());
    }
    image = image.convertToFormat(QImage::Format_RGBA8888);

    const std::vector<uchar> data(image.bits(), image.bits() + image.sizeInBytes());
    const geometrize::Bitmap bitmap(image.width(), image.height(), data);

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

bool writeImage(const geometrize::Bitmap& bitmap, const std::filesystem::path& filePath)
{
    if(bitmap.getWidth() == 0 || bitmap.getHeight() == 0) {
        throw std::runtime_error("Bad bitmap data");
    }

    QImage image(bitmap.getDataRef().data(), bitmap.getWidth(), bitmap.getHeight(), QImage::Format_RGBA8888);

    return image.save(QString::fromStdString(filePath.string()));
}

std::vector<std::filesystem::path> filepathsForDirectory(const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> filepaths;
    QDirIterator it(QString::fromStdString(directory.string()), QDir::Files);
    while(it.hasNext()) {
        filepaths.push_back(it.next().toStdString());
    }
    return filepaths;
}

std::filesystem::path removeExtension(const std::filesystem::path& path)
{
    return path.stem();
}

std::string replaceString(std::string subject, const std::string& search, const std::string& replace)
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
