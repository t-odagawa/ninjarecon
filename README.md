# Tools for track matching between WAGASCI/BabyMIND & NINJA at J-PARC (v0.0.1)

This is a set of tools for (muon) track matching between WAGASCI/BabyMIND detectors
and NINJA detectors (scintillation tracker only?).

## External dependencies (same as WAGASCI+BabyMIND Geant4 simulation)

Below is a copy of README.md in the MC simulation software written by Giorgio Pintaudi.

### GEANT4

[GEANT4](http://heant4.cern/ch/) is a toolkit for the simulation of
the passage of particles through matter developed by CERN.
Geant4 v10.5.0+ is recommended.

### ROOT

[ROOT](https://root.cern.ch/) is an object-oriented program and
library developed by CERN. ROOT 6.20+ is recommended.

### BOOST

[BOOST](https://www.boost.org/) is a set of libraries for the C++
programming language that provides support for tasks and structures
such as linear algebra, pseudorandom number generation,
multithreading, image processing, regular expressions, and unit
testing. Boost 1.53+ is recommended.

### Wagasci BabyMIND Monte Carlo
[Wagasci BabyMIND Monte Carlo](https://git.t2k.org/wagasci_babymind/wagasci-babymind-monte-carlo) is a developed Monte Carlo simulation software for the WAGASCI-BabyMIND experiment.
It includes necessary libraries for the analyses.
Wagasci BabyMIND Monte Carlo v0.1.12+ is recommended.

### Wagasci BabyMIND event display
[Wagasci BabyMIND event display](https://git.t2k.org/wagasci_babymind/wagasci-babymind-event-display) is an event display tool for the WAGASCI-BabyMIND experiment.
Wagasci BabyMIND event display v0.1.0+ is recommended.

### Wagasce Reconstruction

## Installation

Only the following operative systems are tested and supported:
 - CentOS 7 (KEKCC)
 - MacOS 10.15

To install the software with all its dependencies, use a bash shell script `install.bash` in Wagasci BabyMIND Monte Carlo.
For further info about the software installation, refer to the [WAGASCI BabyMIND Monte Carlo gitlab](https://git.t2k.org/wagasci_babymind/wagasci-babymind-monte-carlo).

To compile this NINJA reconstruction software,
```shell script
mkdir build
cd build
cmake .. -DB2MC_PATH=<path/to/your/wagasci/mc/installation> \
  -DEVENT_DISPLAY_PATH=<path/to/your/wagasci/event/display/installation> \
  -DCMAKE_INSTALL_PREFIX=<path/to/your/ninja/recon/installation>
make
make install
```

### KEKCC

For further info about how to create an account and use the KEKCC
cloud computing servers, refer to the [NINJA
wiki](https://www-he.scphys.kyoto-u.ac.jp/research/Neutrino/ninja/dokuwiki/doku.php?id=software:howto:kekcc).

To recompile the package on KEKCC, move to the `build` folder and type:
```shell script
CC=$(command -v gcc) CXX=$(command -v g++) \
  BOOST_ROOT=<path/to/your/boost/installation> \
  cmake3 -DCMAKE_INSTALL_PREFIX=<path/to/your/ninja/recon/installation> ..
```

## Usage

### Hit Converter

Open a terminal and type:

```shell script

```



