all:
	cmake . -DCMAKE_TOOLCHAIN_FILE=/home/niku/Android/NDKs/android-ndk-r21d/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -B build
	cmake --build build

	#adb push build/a64dbg /data/local/tmp

