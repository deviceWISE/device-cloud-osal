# Building the OSAL

## Manually Building with GCC

 **No longer supported! Use CMake instead**

## Building with CMake

### POSIX
 1. Create a build folder and cd into it
 2. Run `cmake ..`
 3. Run `make`

### Windows
 1. Create a build folder and cd into it
 2. Run `cmake ..`
 3. Run Visual Studio as Administrator
 4. Open the newly generated `OSAL.sln`
 5. Run the Build action on the `INSTALL` component