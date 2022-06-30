.build_aarch64:
	cmake . -DCMAKE_TOOLCHAIN_FILE=/home/knistelberger/Android/Ndk/android-ndk-r21d/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -B build
	cmake --build build

	$(PWD)/../../Android/Ndk/android-ndk-r21d/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip build/a64dbg

.build_x64:
	# not supported
	cmake . -B buildx64
	cmake --build buildx64
	strip buildx64/a64dbg

remote: .build_aarch64
	scp build/a64dbg denv:
	ssh denv 'adb push a64dbg /data/local/tmp'

phone: .build_aarch64
	adb push build/a64dbg /data/local/tmp

