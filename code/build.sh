
#!/bin/bash

mkdir -p ../build
pushd ../build
cc ../code/sdl_handmade.cpp -o handmadePenguin -g `sdl2-config --cflags --libs`
popd
