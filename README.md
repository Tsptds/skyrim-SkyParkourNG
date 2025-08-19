# ***SkyParkour V3 NG Procedural Parkour Framework for Skyrim- CommonLibSSE-NG***

## ***Runtime requirements***

- [Skyrim Script Extender (SKSE)](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

## ***Build requirements***

- [CMake](https://cmake.org/)
- [vcpkg](https://vcpkg.io/en/)
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/)
- [CommonLibSSE-NG](https://github.com/alandtse/CommonLibVR/tree/ng)

#### ***CommonLibSSE-NG***

To use CommonLibSSE-NG as a git-submodule instead of overlay-ports, clone it to extern/CommonLibSSE-NG and edit vcpkg.json removing "commonlibsse-ng" and adding its dependencies.

## ***Building***

In `Developer Command Prompt for VS 2022` or `Developer PowerShell for VS 2022`, run:

~~~
git clone
~~~

then

~~~
.\_Build.bat
~~~

or

~~~
.\cmake\build.ps1
~~~

or

~~~
cmake -B build -S . --preset no-vr --fresh
cmake --build build --preset release-novr
~~~

Then get the .dll in build/Release, or the .zip (ready to install using mod manager) in build.

## ***File local.cmake***

CMake will use a file named local.cmake (project root), in this file you can add something like:

~~~
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "bin\\$<CONFIG>\\${PROJECT_NAME}.dll" "C:\\games\\Skyrim\\Data\\SKSE\\Plugins\\${PROJECT_NAME}.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "bin\\$<CONFIG>\\${PROJECT_NAME}.pdb" "C:\\games\\Skyrim\\Data\\SKSE\\Plugins\\${PROJECT_NAME}.pdb"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "C:\\games\\Skyrim\\skse64_loader.exe" WORKING_DIRECTORY "C:\\games\\Skyrim"
)
~~~
