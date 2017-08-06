# LabRender

A simple OpenGL based render engine. Its most interesting feature is that it
has a software configured pipeline architecture. See the assets/pipelines
directory for some example pipeline configurations.

## Usage

See the examples directory for an example.

## Building

LabRender has a few prerequisite libraries. The LabRender library itself
requires GLEW.

The example program also requires GLFW and Assimp. Assimp in turn requires zlib.

At the moment, the cmake scripts assume that they are installed in a directory
named local, where local is a sister directory to the LabRender directory. That
can be changed by modifying the "LOCAL_ROOT" variable in the root cmake script.
This will be cleaned up in the future.

One easy way to install the prerequisites is to use mkvfx, and run it in the
directory containing the LabRender directory.

Run cmake in the root directory picking the generator you wish to use.

Currently, LabRender is tested on Windows 10, with VS2015 or greater.

## Copyright

Copyright 2017, Nick Porcino

## License

The license is BSD 2 clause
