# OpenReflectivity
A C++ implementation of a NEXRAD Level II decoder and viewer (with OpenGL)

This project provides a working implementation of a NEXRAD Level II archive file decoder (currently supports Message 31: REF parsing) as well as an OpenGL-based radar viewer.

## Dependencies
- `cmake`
- `GLFW3`
- `GLEW`
- `OpenGL >= 4.6`
- `ZLIB`
- `BZIP2`

## Building
This application uses `CMake` for the purposes of building.

Clone the repository, create a `build/` directory (e.g. `cmake . -B build`), `cd` into the dir, and then `cmake --build .`

## Acknowledgements
This project took inspiration and was partially based off of
- [nexrad-level-2-data](https://github.com/netbymatt/nexrad-level-2-data)
- [Learn OpenGL](https://learnopengl.com/)
