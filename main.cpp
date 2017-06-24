#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "libbmpread/bmpread.h"

#include "geometrize/shaperesult.h"
#include "geometrize/bitmap/bitmap.h"
#include "geometrize/bitmap/rgba.h"
#include "geometrize/shape/shape.h"
#include "geometrize/shape/shapetypes.h"
#include "geometrize/runner/imagerunner.h"
#include "geometrize/runner/imagerunneroptions.h"

namespace {

geometrize::ImageRunnerOptions generateOptions()
{
    /*
        geometrize::ShapeTypes shapeTypes = geometrize::ShapeTypes::ELLIPSE; ///< The shape types that the image runner shall use.
        std::uint8_t alpha = 128U; ///< The alpha/opacity of the shapes (0-255).
        std::uint32_t shapeCount = 50U; ///< The number of candidate shapes that will be tried per model step.
        std::uint32_t maxShapeMutations = 100U; ///< The maximum number of times each candidate shape will be modified to attempt to find a better fit.
        std::uint32_t seed = 0U; ///< The seed for the random number generators used by the image runner.
        std::uint32_t maxThreads = 0; ///< The maximum number of separate threads for the implementation to use. 0 lets the implementation choose a reasonable number.
    */

    return geometrize::ImageRunnerOptions{};
}

geometrize::Bitmap generateBitmap()
{
	return geometrize::Bitmap(32, 32, geometrize::rgba{12, 12, 12, 12});
}

// Helper function to convert a string to a Geometrize shape type
geometrize::ShapeTypes shapeTypeForName(const std::string& shapeName)
{
    for(const std::pair<geometrize::ShapeTypes, std::string>& p : geometrize::shapeTypeNames) {
        if(p.second == shapeName) {
            return p.first;
        }
    }
    std::cout << "Bad shape type name, defaulting to ellipses \n";
    return geometrize::ELLIPSE;
}

// Helper function to convert a shape type to a human-readable string
std::string shapeNameForType(const geometrize::ShapeTypes type) // Helper function to convert a shape type to a human-readable string
{
    for(const std::pair<geometrize::ShapeTypes, std::string>& p : geometrize::shapeTypeNames) {
        if(p.first == type) {
            return p.second;
        }
    }
    return "unknown";
}

}

int main(int argc, char** argv)
{
	const geometrize::Bitmap bitmap{generateBitmap()};
    const geometrize::ImageRunnerOptions options{generateOptions()};
    geometrize::ImageRunner runner{bitmap};

    const std::size_t totalSteps{100};
    for(std::size_t steps = 0; steps < totalSteps; steps++) {
        const std::vector<geometrize::ShapeResult> shapes{runner.step(options)};
        for(std::size_t i = 0; i < shapes.size(); i++) {
            std::cout << "Added shape " << steps + i << ". Type: " << shapeNameForType(shapes[i].shape->getType()) << " . Score: " << shapes[i].score << "\n";
        }
    }
	
	return 0;
}
