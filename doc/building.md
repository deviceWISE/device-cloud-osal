# Building the OSAL

## Manually Building with GCC

 **No longer supported! Use CMake instead**

## Building with CMake

### POSIX
 1. Create a build folder and cd into it
 2. Run `cmake <path to osal repo>`
 3. Run `make`

To create an installable package, run `make package`.

To install the library, there are two options:
 1. Use `make package` and install the package via `apt` or similar manager
 2. Run `make install` 

### Windows
 1. Create a build folder and cd into it
 2. Run `cmake <path to osal repo>`
 3. Run Visual Studio as Administrator
 4. Open the newly generated `OSAL.sln`
 5. Run the Build action on the
     `ALL_BUILD` component

To create a .zip package of the library, run the Build action on the `PACKAGE`
component.

To install the library, run the Build action on the `INSTALL` component.

 ## Documentation
 This library is setup to use Doxygen to generate documentation. To use it,
 add Doxygen comments inline as normal and generate the documentation
 using `make doc` on Linux or by building `DOC` on Windows.