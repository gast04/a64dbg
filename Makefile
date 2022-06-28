.build:
	cmake . -DCMAKE_TOOLCHAIN_FILE=/home/niku/Android/NDKs/android-ndk-r21d/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -B build
	cmake --build build

	$(PWD)/../../Android/NDKs/android-ndk-r21d/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip build/a64dbg

remote: .build
	scp build/a64dbg denv:
	ssh denv 'adb push a64dbg /data/local/tmp'

phone: .build
	adb push build/a64dbg /data/local/tmp

