# WriteBoost RCU: An RCU Library that Enhances Write-Side Performance

## Build and Run

```sh
# install dependencies
sudo sh install-deps.sh
conan install . --build=missing

# build
cd build/Release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# run
./test/test
```