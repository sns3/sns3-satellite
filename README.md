# Satellite ns-3

Satellite Network Simulator 3 (SNS-3) is a satellite network extension to Network Simulator 3 (ns-3) platform.
SNS-3 was initially developed by Magister Solutions under ESA contact.

# License

SNS-3 is distributed under the GPLv3 license.

Some external modules have been partially or totally integrated to SNS-3:

 * [lorawan](https://github.com/signetlabdei/lorawan), with license GPLv2
 * [SGP4 satellite movement](https://gitlab.inesctec.pt/pmms/ns3-satellite/), with license GPLv2
 * [ISL routing](https://github.com/snkas/hypatia/), with license GPLv2

# Prerequisites

`libxml2-dev` is needed to launch some examples and tests. Install it via

```shell
$ sudo apt install libxml2-dev
```

# Presentation of SNS-3 modules

The installation of SNS-3 requires 3 modules to be installed alongside ns-3:

* `satellite`: main code of satellite module. This is the current repository
* `traffic`: gives additional traffics. Located at: https://github.com/sns3/traffic.git
* `magister-stats`: gives additional functions to handle statistics. Located at: https://github.com/sns3/stats.git

This revision of SNS-3 is compatible with ns-3.48.

If you need to use previous ns-3 versions, you need to make sure that both ns-3 and all additional modules have the same version. Each module is tagged with the corresponding ns-3 version once it has been tested and validated, and we recommend to use the tagged commits.

# Installation Manual

SNS-3 is built as an extension module to the [ns-3](https://www.nsnam.org/) network simulator; so their [installation instructions](https://www.nsnam.org/docs/release/3.48/tutorial/html/getting-started.html) apply, particularly concerning the dependencies. They are repeated here for convenience and proper integration of SNS-3.

There are 2 methods to download and build (S)NS-3:

*  the automated one using [bake](#bake);
*  the manual one using [CMake](#cmake).

The CMake method is recommended, as bake is not always compatible with the latest ns-3 version. The bake version is still presented at the end of this README, but without guarantee of success.

## CMake

If you wish to have finer control over what is being compiled, you can handle the download process of the dependencies yourself and use CMake directly to build ns-3.

You will need to:

*  get ns-3 (by [cloning it using git](https://gitlab.com/nsnam/ns-3-dev.git));
```shell
$ git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3.48
```

*  get the `satellite` module (by [cloning it using git](https://github.com/SNS-3/SNS-3-satellite));
```shell
$ cd ns-3.48/contrib
$ git clone https://github.com/sns3/sns3-satellite.git satellite
```
*  get the `traffic` and `magister-stats` modules (needed until they are integrated into ns-3) as dependencies of the `satellite` module by cloning them :

```shell
$ git clone https://github.com/sns3/traffic.git traffic
$ git clone https://github.com/sns3/stats.git magister-stats
```

*note : When retrieving the **satellite**, **traffic** and **magister-stats** modules, you should put
them under the **ns-3.48/contrib/** folder. You can do so by cloning them directly in this folder,
extracting them here, copying the files afterwards or using symbolic links.*

Make sure all the repositories are using compatible versions. The best way to ensure that is to use the same tag on all repositories. For the latest release:

 * On ns-3 repository:
```shell
$ cd ns-3.48
$ git checkout ns-3.48
```
 * On `satellite`, `traffic` and `magister-stats` repositories:
```shell
$ cd ns-3.48/contrib/[satellite|traffic|magister-stats]
$ git checkout 3.48
```

Then you need to configure CMake and ask it to build ns-3. It will automatically build all modules found in contrib:

```shell
$ cd ns-3.48
$ ./ns3 clean
$ ./ns3 configure --build-profile=optimized --enable-examples --enable-tests
$ ./ns3 build
```

If you want to develop in ns-3, use it in [debug mode](https://www.nsnam.org/docs/release/3.48/tutorial/html/getting-started.html#debugging). It enables debug functionnalities but it is way more slower:

```shell
$ cd ns-3.48
$ ./ns3 clean
$ ./ns3 configure --build-profile=debug --enable-examples --enable-tests
$ ./ns3 build
```

You can also check CMake options to customize it at will:

```shell
$ ./ns3 --help
```

## Post-Compilation

Once you compiled SNS-3 successfully, you will need an extra step before being able to run any simulation: download the data defining the reference scenario of the simulation.

These data are available as a separate repository and bundled as a submodule in SNS-3. You can download them afterwards in the `satellite` repository using:

```shell
$ cd source/ns-3.48/contrib/satellite
$ git submodule update --init --recursive
```

# Testing SNS-3

You can run the unit tests of the ns-3 distribution by running the `./test.py` script:

```shell
$ ./test.py --no-build
```

These tests are run in parallel by ns-3. You should eventually see a report saying that:

```shell
912 of 912 tests passed (912 passed, 0 failed, 0 crashed, 0 valgrind errors)
```

# Running SNS-3

A scenario can be launched as follows:

```shell
$ ./ns3 run <ns3-program>
```

If command line arguments are needed, the command becomes:

```shell
$ ./ns3 run <ns3-program> -- --arg1=value1 --arg2=value2
```

To list all the available command line arguments of a scenario, run:

```shell
$ ./ns3 run <ns3-program> -- --PrintHelp
```

If debug mode is enabled, gdb can be used:

```shell
$ ./ns3 run --gdb <ns3-program>
```

SNS-3 is delivered with several examples. Each one allows to demonstrate one or several functionalities of SNS-3. Statistics are generated in the `data/sims/<ns3-program>` folder.

The main examples are:

 * `sat-cbr-example.cc`: Simple example with CBR traffic
 * `sat-regeneration-example.cc`: Example to test several regeneration modes on satellite: transparent, physical, link or network. By default, all simulations use one transparent satellite
 * `sat-constellation-example.cc`: Example with LEO and GEO satellite constellations. ISLs are used to route packets between satellites, with static routing
 * `sat-vhts-example.cc`: Create a VHTS scenario (high throughputs and high link capacities)
 * `sat-iot-example.cc`: Create an IoT scenario (low throughputs and low link capacities)
 * `sat-logon-example.cc`: Use the logon functionality to log the UTs on the NCC. Traffic on return channels is not send before UT is logged
 * `sat-ncr-example.cc`: Use NCR synchronization between UTs and GWs. UT clock is generally cheap, and need to be resynchronized periodically by the NCC to correctly schedule sending of frames on return channel
 * `sat-lora-example.cc`: Create a scenario with Lora configuration, and on transparent satellite. Lora is a LPWAN protocol developed for IoT
 * `sat-lora-regenerative-example.cc`: Create a scenario with Lora configuration and regenerative satellites


# Annex - Installation using bake

Warning: this is an old installation method, it has not been tested on latest ns-3 and SNS-3 versions, thus there is no garantee of success.

## Preparations

[Bake](http://planete.inria.fr/software/bake/index.html) is a tool developed to simplify the download and install process of ns-3. It can be extended to make it aware of external modules to ns-3 such as SNS-3. You will first need to get bake.

First you need to download Bake using Git, go to where you want Bake to be installed and call

```shell
$ git clone https://gitlab.com/nsnam/bake.git
```

It is advisable to add bake to your path

```shell
$ export BAKE_HOME=`pwd`/bake
$ export PATH=$PATH:$BAKE_HOME
$ export PYTHONPATH=$PYTHONPATH:$BAKE_HOME
```

Before installing ns-3, you will need to tell Bake how to find and download the SNS-3 extension module. To do so, you will have to create a **contrib** folder inside the newly acquired **bake** folder:

```shell
$ cd bake
$ mkdir contrib
$ ls
bake  bakeconf.xml  bake.py  contrib  doc  examples  generate-binary.py  test  TODO
```


and drop the following file **sns3.xml** in this **contrib** folder:

```xml
<configuration>
  <modules>
    <module name="sns3-stats" type="ns-contrib" min_version="ns-3.37">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/stats.git"/>
        <attribute name="module_directory" value="magister-stats"/>
      </source>
      <build type="none">
      </build>
    </module>
    <module name="sns3-traffic" type="ns-contrib" min_version="ns-3.37">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/traffic.git" />
        <attribute name="module_directory" value="traffic"/>
      </source>
      <build type="none">
      </build>
    </module>
    <module name="sns3-satellite" type="ns-contrib" min_version="ns-3.37">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/sns3-satellite.git"/>
        <attribute name="module_directory" value="satellite"/>
        <attribute name="post_download" value="cd $SRCDIR; git submodule update --init --recursive"/>
      </source>
      <depends_on name="sns3-stats" optional="False" />
      <depends_on name="sns3-traffic" optional="False" />
      <build type="none">
      </build>
    </module>
  </modules>
</configuration>
```

This configuration file is used to get all the ns-3 modules needed to compile SNS-3. By default, for each module, bake takes the most recent commit on master. If you want a specific commit for a module, add the following line (with the wanted revision in value) between the flags `<attribute name="url" ...>` and `<attribute name="module_directory" ...>`:

```xml
<attribute name="revision" value="72aa513f43b7687336cf6251d50e81420c41691f"/>
```

It might be necessary to remove the default bake configuration one in order to install SNS-3:
```shell
$ rm bakefile.xml
```

Now you’re ready to use bake.

## Installation

Now that everything is in place, you can tell bake that you want to install SNS-3 (i.e.: `ns-3` plus the `sns3-satellite` module):

```shell
$ ./bake.py configure -e ns-3.48 -e sns3-satellite -e sns3-stats -e sns3-traffic
$ ./bake.py deploy
```

This will download the needed dependencies into a `source` folder and call the various build tools on each target.
If bake finds that tools are missing on your system to download or build the various dependencies it will warn you
and abort the build process if the dependency wasn't optional. You can ask bake for a summary of the required tools before deploying:

```shell
$ ./bake.py check
```
