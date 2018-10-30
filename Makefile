
all:
	pushd build && cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake .. && ninja && popd

clean:
	rm -rf build/*

test:
	pushd build && cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DBUILD_TESTING=ON .. && ninja test && popd
