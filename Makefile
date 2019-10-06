
all:
	cd build && cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DPSKD=24422442 -DCOAP_PSK=sweethomecontrol .. && ninja

clean:
	rm -rf build/*

test:
	pushd build && cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DBUILD_TESTING=ON .. && ninja test && popd
