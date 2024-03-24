#!/bin/sh

set -eu

build_dir=build-tidy
if [ ! -d "$build_dir" ]
then
	mkdir "$build_dir"
	cd "$build_dir"
	cmake -G Ninja -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-warnings-as-errors=*" -DCMAKE_C_CLANG_TIDY="clang-tidy;-warnings-as-errors=*" -DCMAKE_BUILD_TYPE=Debug -Werror=dev -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=. -DDOWNLOAD_GTEST=ON ..
	cmake --build . --config Debug --target everything -- -k 0 || true
	cd ..
fi

[ ! -f .clang-tidy ] && [ -f HACK ] && mv HACK .clang-tidy
mv .clang-tidy HACK
restore() {
	mv HACK .clang-tidy
}
trap restore EXIT

log() {
	printf '[*] %s\n' "$1"
}

find ./src/ \
	-type f \
	-not \( \
		-path './src/test/*' -o \
		-path './src/tools/*' \) \
	\( \
		-name '*.cpp' -o \
		-name '*.c' -o \
		-name '*.h' \
	\) \
	| while read -r source_file
	do
		# TODO: use gnu parralel
		log "checking source_file=$source_file"
		clang-tidy --config-file=scripts/clang-tidy-camel-case.yml "$source_file" -p "$build_dir"
	done
