cd vcpkg
if not exist "%0\..\vcpkg\vcpkg.exe" (
    cmd /c bootstrap-vcpkg.bat
)
vcpkg.exe install boost-python:x64-windows
cd ..
mkdir build
cd build
cmake -A x64 -DTARGET_PLATFORM=%1 -DCMAKE_INSTALL_PREFIX=../pyomvbb/omvbb -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DApproxMVBB_BUILD_LIBRARY_STATIC=OFF ..
cmake --build . --target install
cd ..

