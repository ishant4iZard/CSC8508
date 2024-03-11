mkdir Build
cmake.exe -DCMAKE_TOOLCHAIN_FILE="%SCE_ROOT_DIR%\Prospero\Tools\CMake\PS5.cmake" -A Prospero %* -DDEBUG_ASSET_ROOT:STRING=/app0/Assets/ -B ./Build/PS5 -S .
