[![Geometrize fuzzing logo](https://github.com/Tw1ddle/geometrize-lib-fuzzing/blob/master/screenshots/logo.png?raw=true "Geometrize - library for geometrizing images into geometric primitives fuzzing logo")](http://www.geometrize.co.uk)

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](https://github.com/Tw1ddle/geometrize-lib-fuzzing/blob/master/LICENSE)
[![Build Status Badge](https://ci.appveyor.com/api/projects/status/github/Tw1ddle/geometrize-lib-fuzzing)](https://ci.appveyor.com/project/Tw1ddle/geometrize-lib-fuzzing)

This is a [fuzzing](https://en.wikipedia.org/wiki/Fuzzing) test program for the [Geometrize](https://github.com/Tw1ddle/geometrize-lib) library, a tool for geometrizing images into geometric primitives.

This program ensures that Geometrize handles all sorts of unusual data. It also serves as a basic sample program to demonstrate Geometrize.

[![Geometrized Prairie Dog](https://github.com/Tw1ddle/geometrize-lib-fuzzing/blob/master/screenshots/prairie_dog_lines_tris_and_ellipses.jpg?raw=true "Prairie Dog, 200 ellipses, 1000 polylines and 100 triangles")](http://www.geometrize.co.uk)


## Usage

Build and run the project. Progress is reported in the console. The program should throw an error and quit if something goes wrong.

Currently, the program takes images from the input data folder, geometrizes them using random settings, and saves the results to the output data folder. It also merges input images together to create new test cases.

## Notes
 * Got an idea or suggestion? Open an issue on GitHub, or send Sam a message on [Twitter](https://twitter.com/Sam_Twidale).