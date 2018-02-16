[![Coverity Scan Build Status](https://scan.coverity.com/projects/15035/badge.svg)](https://scan.coverity.com/projects/15035)

# Operating System Abstraction Layer

This repository is abstracts operating system calls into a library.  This allows
for easier porting of the Helix Device Cloud source code to other operating
systems, as well as allowing test teams to build applications without the need
to worry about low-level operating system calls.

This library is a work-in-progress, and more calls will be added as required,
and current calls will have tests built for them to improve reliability.

## Targets
The current targets of the library are: linux, windows 8/10, and eventually
VxWorks and Android.

