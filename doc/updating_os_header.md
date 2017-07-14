# Updating os.h
## Preface
os.h is now dynamically generated at build-time by the scripts found in 
build-sys/header/:
* generate_linux.sh runs on Linux-based systems and will create the header for
  Android, VxWorks, and generic Linux systems
* generate_windows.bat runs on Windows-based systems and will create the header
  for Windows systems

Also in that directory are three .in files which contain header content that is
common to all operating systems. os_h_wrap.in is the header used for building
the library without using macro functions.

Generating will assemble the files in the following order:
 1. os_h_top.in
 2. os_<system\>.h (for Android and VxWorks systems)
 3. os_<platform\>.h (where platform is either 'win' or 'posix')
 4. os_<platform\>_macros.h (for builds that use macro functions)
 4. os_h_bot.in

## Adding OS-Specific Declarations
For OS-specific declarations (ie. where the declarations is different on
different systems, such as when using a macro on one but not others), put the
declaration in the corresponding header in the os/ directory.

When declaring a macro function, you must also create a wrapper function for use
in macro-less builds. Add your macro to os/os_<platform\>_macros.h, the wrapped
declaration to build-sys/os_h_wrap.in, and the wrapped definition to
os/os_<platform\>_wrappers.c.

For example, if `myfunction` is being added and is to be a macro on POSIX but 
not on Windows:

 * Add `#define myfunction() syscall()` to os/os_posix_macros.h
 * Add `OS_API OS_SECTION void myfunction();` to build-sys/header/os_h_wrap.in
 * Add `OS_API OS_SECTION void myfunction();` to os/os_win.h
 * Add your function definition to both os/os_win.c and os/os_posix_wrappers.c

## Adding Common Declarations
For declarations that are the same on all sytems, place the new delcaration in
build-sys/header/os_h_bot.in in the appropriate section.

## Adding Common Types/Definitions
For the most part, new types and definitions that are common to all systems will
go into build-sys/header/os_h_top.in. However, there may be times where it is
better to place these types/definitions after the OS-specific headers are
included. In this case, they can be placed at the top of
build-sys/header/os_h_bot.in.