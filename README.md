# tool-src

This repository contains the source code to a number of utilities I use to help with my reverse engineering efforts, however they are are useful for general build activities.

See the doc directory for information on each of the utilities.

Originally the code was distributed as part of my [Intel80Tools](https://github.com/ogdenpm/intel80tools) repository, however I have refactored into its own repository as a cleaner option, as the number of tools increases. Prebuilt Windows 32bit binaries of these tools are still distributed as part of the [Intel80Tools](https://github.com/ogdenpm/intel80tools) repository. 

Visual studio solution files are provided for all the tools,  but most should compile under linux/unix with little or no modification, although disIntelLib uses C#.

Note if you get a warning message when building  files, it may be because you are using an older or possibly newer version than I have set the project to. Visual Studio provides simple options to retarget to any version you have.
Also the first build of disIntelLIb may generate an error as it cannot find the auto generated files. Future attempts should however work.

There are also a couple of Windows command files from my [versionTools](https://github.com/ogdenpm/versionTools) repository. Of these version.cmd is the main one that creates the version numbers and perl replacement can be found in the [versionTools](https://github.com/ogdenpm/versionTools) repository. The other should be replaced with your own installation scripts.

------

```
Updated by Mark Ogden 12-Oct-2020
```

