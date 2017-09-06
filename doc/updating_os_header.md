# Updating os.h
## Preface
os.h is now dynamically generated at build-time by the script found in the "os"
subdirectory.  In that directory os a os.h.in file which contain header
content that is common to all operating systems.  Operating system information
will be replaced where the symbol `@OS_FUNCTION_DEF@` is defined.

Depending on the operating system, the appropriate header files
`os_<system>.h os_<platform>.h` will be stripped of C defines an included.
Wehre `<system>` is one of: `linux`, `vxworks` or `android`, if applicable,
and `<platform>` is either `win` or `posix`.

## Adding OS-Specific Declarations
For OS-specific declarations (ie. where the declarations is different on
different systems, such as when using a macro on one but not others), put the
declaration in the corresponding header in the os/ directory.

When declaring a macro function, you must wrap the function inside the
appropriate macro definition.

## Adding Common Declarations
For declarations that are the same on all sytems, place the new delcaration in
'os/os.h.in' in the appropriate section.

## Adding Common Types/Definitions
For the most part, new types and definitions that are common to all systems will
go into 'os/os.h.in'. However, there may be times where it is
better to place these types/definitions after the OS-specific headers are
included.
