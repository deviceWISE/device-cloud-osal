# Building the OSAL

## Manually Building with GCC

 **No longer supported! Use CMake instead**

## Building with CMake

### Build Options
Some options are available to be set when running CMake by adding 
`-D<option>=<value>` to your CMake command.
Below is the list of currently used options and their valid values:
* `OSAL_WRAP`: Defines whether to build a regular or "wrapped" library.
  * `0` - regular build
  * `1` - macro-less ("wrapped") methods
* `OSAL_TARGET`: Sets what platform to target when building. *Currently this
  only affects POSIX builds.* If left blank or an invalid string is provided,
  the library will be built for generic POSIX targets.
  * `android` - build for Android systems
  * `vxworks` - build for VxWorks systems

### Macro-less Build
To build the library _without_ using macro functions (for running unit tests, 
etc), add `-DOSAL_WRAP=1` to your CMake command (see below).

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