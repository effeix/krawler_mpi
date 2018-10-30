Clean all (must be inside root)
rm -rf build/*

Compile all (must be inside build/)
/usr/bin/cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -G 'Unix Makefiles' /path/until/before/build/

Make (must be inside build/)
make
