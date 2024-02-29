# tool-src

This repository contains the source code to a number of utilities I use to help with my reverse engineering efforts, however they are are useful for general build activities.

See the doc directory for information on each of the utilities.

Originally the code was distributed as part of my [Intel80Tools](https://github.com/ogdenpm/intel80tools) repository, however I have refactored into its own repository as a cleaner option, as the number of tools increases. Prebuilt Windows binaries of these tools are still distributed as part of the [Intel80Tools](https://github.com/ogdenpm/intel80tools) repository. 

Visual studio solution files are provided for all the tools. If you get a warning message when building  files, it may be because you are using an older or possibly newer version than I have set the project to. Visual Studio provides simple options to retarget to any version you have.

In the Linux directory there are makefile to build all the tools under Linux using GCC, with the exception of disIntelLib which is a C# program that currently builds under the older Windows .Net framework. At some point I will update to use .NET core. The makefiles support the following targets

all - default does a normal build but does not update the version number
clean - clean .o files
distclean - also deletes the target file
publish - as per all but also updates the version number
rebuild - runs distclean followed by make all

Note, the reason for the separate publish target is that when mounting a windows file system under WSL, the getVersion tools run slow as they use git.

There are also some  command files from my [versionTools](https://github.com/ogdenpm/versionTools) repository. Of these getVersion.cmd and getVersion.pl are the main ones that are used to create the version numbers. See the versionTools documentation in the Scripts directory for how to use the install.cmd, or replace with your own solution.

------

```
Updated by Mark Ogden 29-Feb-2024
```

