# Satellite ns3

Satellite Network Simulator 3 (SNS3) is a satellite network extension to Network Simulator 3 (ns-3) platform.
SNS3 was initially developed by Magister Solutions under ESA contact.

# License

SNS3 is distributed under the GPLv3 license.

# Installation Manual

SNS3 is built as an extension module to the [NS3](https://www.nsnam.org/) network simulator; so their [installation instructions](https://www.nsnam.org/docs/release/3.29/tutorial/html/getting-started.html) apply, particularly concerning the dependencies. They are repeated here for convenience and proper integration of SNS3.

There are 2 methods to download and build (S)NS3:

*  the automated one using [bake](#bake);
*  the manual one using [waf](#waf).

## Bake

### Preparations


[Bake](http://planete.inria.fr/software/bake/index.html) is a tool developed to simplify the download and install process of NS3. It can be extended to make it aware of external modules to NS3 such as SNS3. You will first need to get bake.

First you need to download Bake using Git, go to where you want Bake to be installed and call 

```shell
$ git clone https://gitlab.com/nsnam/bake
```

It is advisable to add bake to your path

```shell
$ export BAKE_HOME=`pwd`/bake 
$ export PATH=$PATH:$BAKE_HOME
$ export PYTHONPATH=$PYTHONPATH:$BAKE_HOME
```

Before installing NS3, you will need to tell Bake how to find and download the SNS3 extension module. To do so, you will have to create a **contrib** folder inside the newly acquired **bake** folder:

```shell
$ cd bake
$ mkdir contrib
$ ls
bake  bakeconf.xml  bake.py  build  contrib  doc  examples  generate-binary.py  test  TODO
```


and drop the following file **sns3.xml** in this **contrib** folder:

```xml
<configuration>
  <modules>
    <module name="sns3-satellite" type="ns-contrib" min_version="ns-3.29">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/sns3-satellite.git"/>
        <attribute name="module_directory" value="satellite"/>
      </source>
      <build type="none">
      </build>
    </module>
    <module name="sns3-stats" type="ns-contrib" min_version="ns-3.29">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/stats.git"/>
        <attribute name="module_directory" value="magister-stats"/>
      </source>
      <build type="none">
      </build>
    </module>
    <module name="sns3-traffic" type="ns-contrib" min_version="ns-3.29">
      <source type="git">
        <attribute name="url" value="https://github.com/sns3/traffic.git" />
        <attribute name="module_directory" value="traffic"/>
      </source>
      <build type="none">
      </build>
    </module>
  </modules>
</configuration>
```
It might be necessary to remove the default bake configuration one in order to install sns3:
```shell
$ rm bakefile.xml
```

Now you’re ready to use bake.

### Installation

Now that everything is in place, you can tell bake that you want to install SNS3 (i.e.: ''ns-3'' plus the ''sns3-satellite'' module):

```shell
$ ./bake.py configure -e ns-3.29 -e sns3-satellite -e sns3-stats -e sns3-traffic
$ ./bake.py deploy
```

This will download the needed dependencies into a ''source'' folder and call the various build tools on each target. 
If bake finds that tools are missing on your system to download or build the various dependencies it will warn you 
and abort the build process if the dependency wasn't optional. You can ask bake for a summary of the required tools before deploying:

```shell
$ ./bake.py check
```

## Waf

Behind the scene, bake delegates to [waf](https://waf.io/apidocs/index.html) the build of NS3. If you wish to have finer 
control over what is being compiled, you can handle the download process of the dependencies yourself and use waf directly to build NS3.

You will need to:


*  get NS3 (either by [downloading](https://www.nsnam.org/release/) it, [cloning it using mercurial](http://code.nsnam.org/) or [cloning it using git](https://gitlab.com/nsnam/ns-3-dev.git));
```shell
$ git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3.29

```

*  get the ''satellite'' module (by [cloning it using git](https://github.com/sns3/sns3-satellite));
```shell
$ cd ns-3.29/contrib
$ git clone https://github.com/sns3/sns3-satellite satellite

```
*  get the ''traffic'' and ''magister-stats'' modules (needed until they are integrated into NS3) as dependencies of the ''satellite'' module by cloning them :


```shell
$ git clone https://github.com/sns3/traffic.git traffic
$ git clone https://github.com/sns3/stats.git magister-stats
    
```

*note : When retrieving the **satellite**, **traffic** and **magister-stats** modules, you should put 
them under the **ns-3.29/contrib/** folder. You can do so by cloning them directly in this folder, 
extracting them here, copying the files afterwards or using symbolic links.*

Then you need to configure waf and ask it to build NS3. It will automatically build all modules found in contrib:

```shell
$ cd ns-3.29
$ ./waf configure -d optimized --enable-examples --enable-tests
$ ./waf build -j 6
```
You can also check waf options to customize it at will:


```shell
$ ./waf --help
```

## Post-Compilation

Once you compiled SNS-3 successfully, you will need an extra step before being able to run any simulation: download the data defining the reference scenario of the simulation.

These data are available as a separate repository and bundled as a submodule in SNS-3. You can download them afterwards in the ''satellite'' repository using:

```shell
$ cd source/ns-3.29/contrib/satellite
$ git submodule update --init --recursive
```
