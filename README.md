# Spexpert

Program which controls measurements on Spex spectrometer in our laboratory.


## Requirements

* `Windows` operating system
* `git` – to clone the `spexpert` repository. You can get git from [here][git].
* `Qt` > 5 – to compile the program. You can get it from [here][qt].
* `MinGW` – it can be installed with Qt (32bit compiler) or downloaded for example with `MSYS2` suite from
  [here][msys2]. From MSYS2 terminal, you can also download and install Qt library for 64 mingw
  ([description][qtmsys2]).
* `CMake` build system, you can get it from [here][cmake].
* `Doxygen` – for documentation compilation, you can get it from [here][doxygen].
* `dot` – for graphs in documentation, it is part of the GraphViz library, you can get it from [here][graphviz].
* `WinSpec` – controlls detector and spectrograph
* `8SMC1-USBh` – microstep controller driver
* K8090 relay card drivers – [download][k8090download]

Make sure that the CMake, Doxygen and GraphViz binaries are in the `PATH` variable.

In our case, the spectrometer has to be connected to 32bit Windows XP, so you can't use the last versions of the
dependencies, the last version of `git` supporting Windows XP is `2.10.0`, `Qt` can't use ANGLE so, the last compiled
officialy suplied version with OpenGL is `5.4.2` and last supported `python` version is `3.4`.


## Getting Started

To get you started you can clone the `spexpert` repository and compile program with `Qt`. The documentation is created by
`Doxygen` and `dot` program from `GraphViz` library.


### Obtaining `spexpert` application

Clone the `spexpert` repository using git:

```
git clone https://github.com/biomolecules/spexpert.git
```

If you just want to use `spexpert` without the commit history then you can do:

```
git clone --depth=1 https://github.com/biomolecules/spexpert.git
```

The `depth=1` tells git to only pull down one commit worth of historical data.

Then go to project folder (`cd spexpert`) and checkout to develop branch:
```
git checkout develop
```
and then download submodules:
```
git submodule update --init --recursive
```


### Compilation


#### Command line compilation

Open the Qt Console (it should run `qtenv2.bat` to setup environment, for example
`C:\WINDOWS\system32\cmd.exe /A /Q /K C:\Qt\Qt5.4.2_mingw\5.4\mingw491_32\bin\qtenv2.bat`). Then navigate to
the project directory and run
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_INSTALL_PREFIX=..
```

You can skip `CMAKE_INSTALL_PREFIX`. Application is then installed to default destination ("C:\Program Files" on
Windows, "/usr/local" on Linux).

Then run your `make` command, for example (`-j` flag enables compilation paralelization)
```
mingw32-make -j2
mingw32-make doc       # optional if you want to make documentation
mingw32-make install   # optional if you want to install the application, see
# above
```

After make process finishes, go to the bin directory and try to run the program
```
cd C:\path\where\you\want\the\application\installed\bin
spexpert.exe
```


#### Compilation using Qt Creator

Open the CMakeFiles project file in the top directory and cofigure it for appropriate compiler. Add arguments as
discussed above.

Then compile the documentation by executing
```
doxygen Doxyfile
```
from command line from the build project folder.


[git]: https://git-scm.com/
[qt]: https://www.qt.io/
[msys2]: http://www.msys2.org/
[qtmsys2]: https://wiki.qt.io/MSYS2
[cmake]: https://cmake.org/download/
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[graphviz]: http://graphviz.org/
[k8090download]: http://www.vellemanusa.com/downloads/files/downloads/k8090_vm8090_rev1.zip
