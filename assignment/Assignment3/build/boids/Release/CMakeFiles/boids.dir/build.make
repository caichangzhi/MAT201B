# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.16.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.16.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/changzhicai/Desktop/MAT201B/allolib_playground

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release

# Include any dependencies generated for this target.
include CMakeFiles/boids.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/boids.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/boids.dir/flags.make

CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o: CMakeFiles/boids.dir/flags.make
CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o: ../../../boids.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o -c /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/boids.cpp

CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/boids.cpp > CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.i

CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/boids.cpp -o CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.s

# Object files for target boids
boids_OBJECTS = \
"CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o"

# External object files for target boids
boids_EXTERNAL_OBJECTS =

../../../bin/boids: CMakeFiles/boids.dir/caichangzhi/assignment/Assignment3/boids.cpp.o
../../../bin/boids: CMakeFiles/boids.dir/build.make
../../../bin/boids: ../../../../../../allolib/build/Release/libal.a
../../../bin/boids: ../../../../../../allolib/build/al_ext/openvr/libal_openvr.a
../../../bin/boids: ../../../../../../allolib/build/al_ext/soundfile/libal_soundfile.a
../../../bin/boids: ../../../../../../allolib/build/al_ext/statedistribution/libal_statedistribution.a
../../../bin/boids: ../../../../../../allolib/build/Release/libal.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/Gamma/lib/libGamma.a
../../../bin/boids: /usr/local/lib/libsndfile.dylib
../../../bin/boids: ../../../../../../allolib/build/Release/external/glfw/src/libglfw3.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/glad/libglad.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/rtaudio/librtaudio.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/rtmidi/librtmidi.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/libimgui.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/liboscpack.a
../../../bin/boids: ../../../../../../allolib/build/Release/external/libserial.a
../../../bin/boids: CMakeFiles/boids.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/boids"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/boids.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/boids.dir/build: ../../../bin/boids

.PHONY : CMakeFiles/boids.dir/build

CMakeFiles/boids.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/boids.dir/cmake_clean.cmake
.PHONY : CMakeFiles/boids.dir/clean

CMakeFiles/boids.dir/depend:
	cd /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/changzhicai/Desktop/MAT201B/allolib_playground /Users/changzhicai/Desktop/MAT201B/allolib_playground /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release /Users/changzhicai/Desktop/MAT201B/allolib_playground/caichangzhi/assignment/Assignment3/build/boids/Release/CMakeFiles/boids.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/boids.dir/depend

