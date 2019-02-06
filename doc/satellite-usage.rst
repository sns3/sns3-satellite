++++++++++++++++++
User Documentation
++++++++++++++++++

Usage
*****

Building Satellite Module
=========================

Satellite module uses a distributed ns-3 approach enabled by bake tool. 
Satellite module does not currently support Mac OS X due to SatEnvVariables class using Linux OS specific commands. 

Overview
########

Setting up a working version of SNS3 requires the following parts:

- Bake
- The simulator (NS-3, satellite modules, etc.)
- Data package

The steps below will guide you to quickly set up the parts.

Bake
####

Bake is a small tool that automates the deployment of NS-3, other related modules, and their dependencies. 
Bake is an open source tool, so we shall start by downloading its source. Open a terminal and tell Mercurial 
to download Bake from its official repository.
::

  $ hg clone http://code.nsnam.org/bake

You will be shown an output similar to the following
::

  destination directory: bake
  requesting all changes
  adding changesets
  adding manifests
  adding file changes
  added 358 changesets with 817 changes to 63 files
  updating to branch default
  resolving manifests
  getting .hgignore
  getting .project
  ...
  getting test/test.xml
  45 files updated, 0 files merged, 0 files removed, 0 files unresolved

Bake is now downloaded to ``bake`` directory.
::

  $ cd bake

Now go to `the SNS3 wiki`__  and check the `sns3.xml`__ file there, download it, and copy it into
your ``bake/contrib/`` directory. This ``sns3.xml`` contains information of new modules related to SNS3.

.. _sns3_wiki: https://wiki.net4sat.org/doku.php?id=sns3:index

__ sns3_wiki_

.. _sns3_bakefile: https://wiki.net4sat.org/doku.php?do=export_code&id=sns3:manuals:install:index&codeblock=3

__ sns3_bakefile_

The simulator
#############

We can now proceed to download the simulator.

Then we use Bake to take care of the download. In ``bake`` directory, run the following commands.
::

  ./bake.py configure --enable ns-3.28 --enable sns3-satellite
  ./bake.py download 


The last command usually takes some time, especially while downloading ns-3.28.

The simulator is now downloaded to the ``bake/source/ns-3.28`` directory.
::

  $ cd source/ns-3.28

You're now in the root directory of NS-3. If you wish, we can proceed to configure and build the simulator 
here.
::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf build

The build should take some time. But keep in mind that SNS3 is not functioning yet without the data package. 
This is explained in the next section.

You can also combine both steps (download + build) into a single step by running:
::

  ./bake.py configure --enable ns-3.28 --enable sns3-satellite
  ./bake.py deploy

Lastly, you can check your system for missing dependencies before downloading the sources using:
::

  ./bake.py configure --enable ns-3.28 --enable sns3-satellite
  ./bake.py check


Data package
############

Data package is a collection of static configuration-like input files for the satellite module. The files are 
distributed separately from the satellite module because of its huge size (around 1 GB). The data package contains 
the following information:

- Antenna patterns
- External fading traces
- Fading traces
- Interference traces
- Link results
- Rx power traces
- SINR measurement error files
- UT positions

The input files are placed inside the data directory of the satellite module 
(i.e., contrib/satellite/data directory). 

Data package is provided as a submodule of the main satellite module. You can ask Git to
download it using the following command:
::

  $ cd contrib/satellite
  $ git submodule update --init

SNS3 is now properly initialized. 


Helpers
=======

.. _fig-satellite-helper-structure:

.. figure:: figures/satellite-helper-structure.*
	:scale: 70 %
	:align: center

	Satellite helper structure

The satellite module adopts a set of hierarchical satellite helper classes, which are used to generate the
satellite simulation scenario: in relation to the used simulation scenario size (number of beams),
number of UTs, terrestrial network access technology, number of terrestrial end users and their
applications. 

Simulation helper
#################

To simplify the process of configuring a custom simulation script, the Satellite module includes a 
``SimulationHelper`` class. Its sole purpose is to help create satellite simulation scripts by defining e.g. 
beams utilized, number of users, weather traces, statistics and certain parameter groups by using simple 
function calls. Simulation helper leaves only installation of applications to nodes to the user. 
Helper structure is presented in :ref:`fig-satellite-helper-structure`.

Using Simulation helper's methods most key attribute groups are configurable by simple function calls. 
However, micromanaging of attributes in NS-3 style using ``Config::SetDefault (...)`` and 
``Object::SetAttribute (...)`` is also possible at the same time with Simulation helper. 

The key configuration methods are presented in :ref:`tab-simulation-helper-configuration`: 

.. _tab-simulation-helper-configuration:

.. table:: Simulation configuration methods

	========================================================================   ====================================================================================================================================================
	Name of method                                                             Description
	========================================================================   ====================================================================================================================================================
	SimulationHelper:: SetDefaultValues                                        Set default values shared by all examples using SimulationHelper. 
	SimulationHelper:: SetBeams                                                Set enabled beams (1-72) as a string.
	SimulationHelper:: SetUtCountPerBeam                                       Set UT count per beam.
	SimulationHelper:: SetUserCountPerUt                                       Set user count per UT.
	SimulationHelper:: SetSimulationTime                                       Set simulation time in seconds. 
	SimulationHelper:: SetIdealPhyParameterization                             Set ideal channel/physical layer parameterization.
	SimulationHelper:: EnableAcm                                               Enable ACM for a simulation direction.
	SimulationHelper:: DisableAcm                                              Disable ACM for a simulation direction.
	SimulationHelper:: DisableAllCapacityAssignmentCategories                  Disable all capacity allocation categories: CRA/VBDC/RBDC.
	SimulationHelper:: EnableOnlyConstantRate                                  Enable only CRA for a given RC index. 
	SimulationHelper:: EnableOnlyRbdc                                          Enable only RBDC for a given RC index.
	SimulationHelper:: EnableOnlyVbdc                                          Enable only VBDC for a given RC index. 
	SimulationHelper:: EnableFca                                               Enable free capacity allocation.
	SimulationHelper:: DisableFca                                              Disable free capacity allocation.
	SimulationHelper:: EnablePeriodicalControlSlots                            Enable periodical control slots.
	SimulationHelper:: EnableArq                                               Enable ARQ.
	SimulationHelper:: DisableRandomAccess                                     Disable random access.
	SimulationHelper:: EnableSlottedAloha                                      Enable slotted ALOHA random access.
	SimulationHelper:: EnableCrdsa                                             Enable CRDSA random access. 
	SimulationHelper:: ConfigureFrame                                          Configure a frame for a certain superframe id (superframe, frame, frame bandwidth, carrier bandwidth, roll-off, carrier spacing, is random access).
	SimulationHelper:: ConfigureFrequencyBands                                 Configure the default setting for the forward and return link frequencies.
	SimulationHelper:: EnableExternalFadingInputTrace                          Enable external fading input.
	SimulationHelper:: EnableOutputTraces                                      Enable all output traces. 
	SimulationHelper:: ConfigureLinkBudget                                     Configure all link budget related attributes. 
	SimulationHelper:: SetErrorModel                                           Set simulation error model and error rate.
	SimulationHelper:: SetInterferenceModel                                    Set simulation interference model.
	========================================================================   ====================================================================================================================================================

:ref:`tab-simulation-helper-control` contains the rest of the 
methods needed to run the simulation. 
Configuration methods listed in :ref:`tab-simulation-helper-configuration` must be called before these.

\ 


.. _tab-simulation-helper-control:

.. table:: Simulation control methods

	========================================================================   ====================================================================================================================================================
	Name of method                                                             Description
	========================================================================   ====================================================================================================================================================
	SimulationHelper:: SetOutputPath                                           Set the output directory for statistics.
	SimulationHelper:: SetOutputTag                                            Alternative for ``SetOutputPath``. Set simulation output tag, which is the basename of the directory where output files are stored.
	SimulationHelper:: CreateSatScenario                                       Create the satellite scenario.
	SimulationHelper:: CreateDefaultStats                                      Create stats collectors. Adjust this method to your needs.
	SimulationHelper:: EnableProgressLogging                                   Enables simulation progress logging to standard output.
	SimulationHelper:: RunSimulation                                           Run the simulation.
	========================================================================   ====================================================================================================================================================


Note, that almost every class of the Satellite module contains some attributes. 
It is encouraged for the user to get to know the attributes in classes he/she focuses on in custom simulations. 
For more information about available attributes, see the following chapters' helper attributes. 
For advanced usage, see the section `Advanced usage and Attributes`_.

Satellite helper 
################

Satellite helper is a main helper class which abstracts all the complexity inside 
sub-helpers, which are presented below. The satellite helper is by default capable of generating
three kinds or scenarios: one spot-beam, full reference system and a user-defined subset
of the full reference system (any user defined amount of spot-beams). However, the
scenario creation always obeys the parameterized reference system. The Satellite helper attributes are presented in 
:ref:`tab-sat-helper`.

\ 

.. _tab-sat-helper:

.. table:: Satellite helper attributes

	=====================================================  ==================================================================================
	Name of attribute                                      Description
	=====================================================  ==================================================================================
	ns3::SatHelper:: UtCount                               Number of UTs per beam.
	ns3::SatHelper:: GwUsers                               Number of gateway users.
	ns3::SatHelper:: UtUsers                               Number of users per UT.
	ns3::SatHelper:: BeamNetworkAddress                    Initial network number to use during allocation of satellite devices. 
	ns3::SatHelper:: BeamNetworkMask                       Network mask to use during allocation of satellite devices. 
	ns3::SatHelper:: GwNetworkAddress                      Initial network number to use during allocation of GW, router, and GW users.
	ns3::SatHelper:: GwNetworkMask                         Network mask to use during allocation of GW, router, and GW users.
	ns3::SatHelper:: UtNetworkAddress                      Initial network number to use during allocation of UT and UT users
	ns3::SatHelper:: UtNetworkMask                         Network mask to use during allocation of UT and UT users.
	ns3::SatHelper:: PacketTraceEnabled                    Packet tracing enable status.
	ns3::SatHelper:: ScenarioCreationTraceEnabled          Scenario creation trace output enable status. 
	ns3::SatHelper:: DetailedScenarioCreationTraceEnabled  Detailed scenario creation trace output enable status.
	ns3::SatHelper:: ScenarioCreationTraceFileName         File name for the scenario creation trace output.
	ns3::SatHelper:: UtCreationTraceFileName               File name for the UT creation trace output.
	ns3::SatHelper:: Creation                              Creation traces. 
	ns3::SatHelper:: CreationSummary                       Creation summary traces. 
	=====================================================  ==================================================================================


Beam helper
###########

Beam helper creates needed ``SatChannel`` and ``SatNetDevice`` objects, with help of other lower
level device helpers, and creates needed GW nodes. The helper assigns IP addresses for
every node connected to satellite network, sets IP routes to nodes for satellite network and
fills Address Resolution Protocol (ARP) caches for satellite network. The Beam helper attributes 
are presented in :ref:`tab-beam-helper`.

\ 

.. _tab-beam-helper:

.. table:: Beam helper attributes

	=================================================================  ==================================================================================
	Name of attribute                                                  Description
	=================================================================  ==================================================================================
	ns3::SatBeamHelper:: CarrierFrequencyConverter                     Callback to convert carrier id to generate frequency.
	ns3::SatBeamHelper:: FadingModel                                   Fading model.
	ns3::SatBeamHelper:: RandomAccessModel                             Random access model. 
	ns3::SatBeamHelper:: RaInterferenceModel                           Interference model for random access. 
	ns3::SatBeamHelper:: RaCollisionModel                              Collision model for random access.
	ns3::SatBeamHelper:: PropagationDelayModel                         Propagation delay model.
	ns3::SatBeamHelper:: ConstantPropagationDelay                      Constant propagation delay.
	ns3::SatBeamHelper:: PrintDetailedInformationToCreationTraces      Print detailed information to creation traces.
	ns3::SatBeamHelper:: CtrlMsgStoreTimeInFwdLink                     Time to store a control message in container for forward link.
	ns3::SatBeamHelper:: CtrlMsgStoreTimeInRtnLink                     Time to store a control message in container for return link.
	ns3::SatBeamHelper:: Creation                                      Creation traces.
	=================================================================  ==================================================================================


GEO helper
##########

GEO helper creates a ``SatNetDevice`` object for GEO satellite node and configures the
satellite switch to deliver packets through satellite node. The GEO helper attributes are presented in :ref:`tab-geo-helper`.

\ 

.. _tab-geo-helper:

.. table:: GEO helper attributes

	=================================================================   ==================================================================================
	Name of attribute                                                   Description
	=================================================================   ==================================================================================
	ns3::SatGeoHelper:: DaFwdLinkInterferenceModel                      Forward link interference model for dedicated access.
	ns3::SatGeoHelper:: DaRtnLinkInterferenceModel                      Return link interference model for dedicated access
	ns3::SatGeoHelper:: Creation                                        Creation traces.  
	=================================================================   ==================================================================================


GW helper
#########

GW helper creates ``SatNetDevice`` objects for GW nodes and attaches them to proper 
``SatChannel`` objects. The GW helper attributes are presented in :ref:`tab-gw-helper`. 

\ 

.. _tab-gw-helper:

.. table:: GW helper attributes

	=================================================================   ==================================================================================
	Name of attribute                                                   Description
	=================================================================   ==================================================================================
	ns3::SatGwHelper:: RtnLinkErrorModel                                Return link error model. 
	ns3::SatGwHelper:: DaRtnLinkInterferenceModel                       Return link interference model for dedicated access
	ns3::SatGwHelper:: EnableChannelEstimationError                     Enable channel estimation error in return link receiver at GW.              
	ns3::SatGwHelper:: Creation                                         Creation traces.  
	=================================================================   ==================================================================================

UT helper
#########

UT helper creates ``SatNetDevice`` objects for UT nodes and attaches them to
proper ``SatChannel`` objects. The UT helper attributes are presented in :ref:`tab-ut-helper`. 

\ 


.. _tab-ut-helper:

.. table:: UT helper attributes


	=================================================================   ==================================================================================
	Name of attribute                                                   Description
	=================================================================   ==================================================================================
	ns3::SatUtHelper:: FwdLinkErrorModel                                Forward link error model. 
	ns3::SatUtHelper:: DaFwdLinkInterferenceModel                       Forward link interference model for dedicated access
	ns3::SatUtHelper:: LowerLayerServiceConf                            Pointer to lower layer service configuration.
	ns3::SatUtHelper:: EnableChannelEstimationError                     Enable channel estimation error in forward link receiver at GW.              
	ns3::SatUtHelper:: UseCrdsaOnlyForControlPackets                    CRDSA utilized only for control packets or also for user data.      
	ns3::SatUtHelper:: Creation                                         Creation traces.  
	=================================================================   ==================================================================================


User helper
###########

User helper creates needed amount of end user nodes for end user networks (user
connected to UTs) and for public network (behind GWs), their access technologies,
channels and IP routes. The helper is also responsible of creating different application
scenarios. The user helper attributes are presented in :ref:`tab-user-helper`.

\ 

.. _tab-user-helper:

.. table:: User helper attributes

	=================================================================   =====================================================================================
	Name of attribute                                                   Description
	=================================================================   =====================================================================================
	ns3::SatUserHelper:: BackboneNetworkType                            Network used between GW and Router, and between Router and Users in operator network.
	ns3::SatUserHelper:: SubscriberNetworkType                          Network used between UTs and Users in subscriber network.
	ns3::SatUserHelper:: Creation                                       Creation traces.  
	=================================================================   =====================================================================================



Output
======

Satellite module is able to print class-specific logs by enabling ns-3 ``LogComponent`` 
objects by different log levels, usually LOG_LEVEL_INFO. For more information about ns-3 log system, please see 
`the ns-3 logging tutorial`__. 

.. _log_tutorial: https://www.nsnam.org/docs/release/3.7/tutorial/tutorial_21.html

__ log_tutorial_

Satellite module supports a set of statistics by using the Data Collection Framework (DCF). 
The available statistics are presented in :ref:`tab-supported-stats`. 

\ 

.. _tab-supported-stats:

.. table:: Supported statistics

	==========================================    ==========================       =================================
	Statistics name                               Applicable link directions       Applicable levels
	==========================================    ==========================       =================================
	Throughput                                    Both                             Application, device, MAC, and PHY
	Packet delay                                  Both                             Application, device, MAC, and PHY
	Signalling load                               Both                             Device
	Queue size (in bytes)                         Both                             LLC
	Queue size (in number of packets)             Both                             LLC
	Capacity request                              Return link                      LLC
	Resources granted                             Forward link                     MAC
	SINR                                          Both                             PHY
	DA packet error                               Both                             PHY
	RA CRDSA packet error                         Return link                      PHY
	RA CRDSA packet collision                     Return link                      PHY
	RA Slotted ALOHA packet error                 Return link                      PHY
	RA Slotted ALOHA packet collision             Return link                      PHY
	Backlogged request                            Forward link                     NCC
	Frame load (in ratio of allocated symbols)    Return link                      NCC
	Frame load (in number of scheduled users)     Return link                      NCC
	Waveform usage                                Return link                      NCC
	==========================================    ==========================       =================================

Statistics framework is disabled by default. To enable it, thereby allowing it to produce output,
users may utilize the ``SatStatsHelperContainer`` class.
The first step is to instantiate the class into an object instance. This is done by passing the
``SatHelper`` instance used in the simulation as an input argument to the constructor, as follows.
::

  Ptr<SatHelper> h = CreateObject<SatHelper> ("Scenario72");
  h->CreateScenario (SatHelper::SIMPLE);
  
  // ... (snip) ...
  
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer>; (h);

Then a statistics type can be enabled by calling a method.
::

  s->AddPerBeamRtnDevDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);

There are a lot of methods to choose from the SatStatsHelperContainer object. Please refer to the
Doxygen documentation section of ``SatStatsHelperContainer`` for the complete list. 

Each statistics type has different range of supported output types.
The text-based statistics, e.g., the capacity request and backlogged request, only support
OUTPUT_SCATTER_FILE type. The rest of the statistics support the following:

- OUTPUT_SCALAR_FILE
- OUTPUT_SCATTER_FILE
- OUTPUT_SCATTER_PLOT

In addition to the above, the following output types apply to packet delay, queue size, resources 
granted, and SINR statistics.

- OUTPUT_HISTOGRAM_FILE
- OUTPUT_PDF_FILE
- OUTPUT_CDF_FILE
- OUTPUT_HISTOGRAM_PLOT
- OUTPUT_PDF_PLOT
- OUTPUT_CDF_PLOT

Note that the output types are divided to either FILE or PLOT group, as indicated by the suffix. The
group determines the type of aggregator to be used. 

Identifier type determines how the statistics are categorized. The possible options are ``GLOBAL`` 
(not categorized at all), ``PER_GW``, ``PER_BEAM``, and ``PER_UT``. Application-level statistics may also
accept ``PER_UT_USER`` as an additional identifier. These options are indicated in the name of each
method.

As the name implies, the ``SatStatsHelperContainer`` object instance acts as a container of several
helpers. Therefore, more than one statistics type can be enabled and become concurrently active
within the same simulation, i.e., allowing users to produce more than one statistics output in one
simulation run.

Advanced Usage and Attributes
=============================

User and feeder links
#####################

User and feeder links are configured by attributes of SatConf. Link bandwidth and frequency can be 
set separately per each link. The :ref:`tab-bandwidth-conf` describes all these attributes. 

\ 

.. _tab-bandwidth-conf:

.. table:: FWD and RTN link bandwidth configuration attributes

	=========================================                          ================================================================================
	Name of the attribute                                              Description
	=========================================                          ================================================================================
	ns3::SatConf:: FwdFeederLinkBandwidth                              Defines bandwidth for the forward feeder link (in Hertz).
	ns3::SatConf:: FwdFeederLinkBaseFrequency                          Defines the lower boundary frequency of the forward feeder link band (in Hertz).  
	ns3::SatConf:: RtnFeederLinkBandwidth                              Defines bandwidth for the return feeder link (in Hertz).
	ns3::SatConf:: RtnFeederLinkBaseFrequency                          Defines the lower boundary frequency of the return feeder link band (in Hertz). 
	ns3::SatConf:: FwdUserLinkBandwidth                                Defines bandwidth for the forward user link (in Hertz).
	ns3::SatConf:: FwdUserLinkBaseFrequency                            Defines the lower boundary frequency of the forward user link band (in Hertz).  
	ns3::SatConf:: RtnUserLinkBandwidth                                Defines bandwidth for the return user link (in Hertz).
	ns3::SatConf:: RtnUserLinkBaseFrequency                            Defines the lower boundary frequency of the return user link band (in Hertz).
	=========================================                          ================================================================================

User link bandwidth is divided to equal channels by attribute ``ns3::SatConf::UserLinkChannels`` for 
both direction, forward and return. Feeder link is divided to channels same way for both direction 
by attribute ``ns3::SatConf::FeederLinkChannels``. Satellite module verifies correctness of the configuration 
by checking that bandwidths of the channels are same for both links in one direction (forward or return). 
In case of error simulation is terminated by causing fatal error.

Return link frame configuration
###############################

Superframe structure for the return link is the same for every channel. Currently the satellite module 
supports only superframe sequence 0 (one sequence). Structure for this sequence can be 
selected among the four superframe configurations. Selection is done by attribute 
``ns3::SatConf::SuperFrameConfForSeq0`` in SatConf class. Superframe structure itself for the 
each selectable configuration is defined by attributes of the each superframe configuration 
objects ``SatSuperframeConf0``, ``SatSuperframeConf1``, ``SatSuperframeConf2`` and 
``SatSuperframeConf3``.
Each of these objects is derived from same abstract base class ``SatSuperframeConf``. 
Purpose is to provide four pre-defined configurations for the superframes devised 
to facilitate user configuration. In other words these classes are exactly same 
except the default values that are assigned to their attributes. If pre-defined values 
are not enough for a simulation purposes they can be overridden by attributes without 
re-compiling simulator. Configuration of superframe by attributes is described in 
`Superframe structure configuration`_. 


Forward link carrier configuration  
##################################

Forward link channels are divided to equal-size carriers using carrier bandwidth defined by attribute 
of the ``SatConf`` object ``ns3::SatConf::FwdCarrierAllocatedBandwidth``. Value of this attribute 
cannot exceed the calculated bandwidth value for forward link channel bandwidths. This ensures that 
there is minimum one carrier available in forward link. Currently the satellite module supports 
only using of the one carrier in forward link per beam. Used carrier is logical first i.e. carrier having 
the lowest center frequency (index or id 0). Carrier spacing and roll-off for the every forward link 
carrier is defined by ``SatConf`` attributes ``ns3::SatConf::FwdCarrierSpacing`` and 
``ns3::SatConf::FwdCarrierRollOff``.


Superframe structure configuration
##################################

Superframe structure that is supported by the satellite module is such that all frames in the superframe are 
constructed according to target duration defined by attribute TargetDuration of the SatSuperframeSeq. 
Superframe structure can have in maximum 10 configurable frames. The number of the actually used 
frames are selected by attribute of the ``SatSuperframeConfX`` (e.g. ``ns3::SatSuperframeConf0::FrameCount``). 
Type of the frame configuration for each frame in superframe is selected by attribute 
``ns3::SatSuperframeConf0::FrameConfigType`` from three supported types. Supported types are 0-2. 
The ``ns3::SatSuperframeConf0::FrameConfigType`` attribute has influence how frame are constructed by 
class ``SatFrameConf`` implementing frame configuration and utilized by ``SatFrameAllocator`` class.
Each of these 10 configurable frames have same configurable attributes, but can be configured individually. 
Only as many configurations as selected by FrameCount attribute has meaning in superframe configuration. 
Frame selected in use are taking in ascending order. E.g. if frame count is 1 then Frame0 is in use, if 
frame count is 2 then Frame0 and Frame1 are in use and so on. Each frame is configured with the 5 different 
attributes of ``SatSuperframeConfX``. The example of these attributes are shown in 
:ref:`tab-superframe-conf` for Frame0 of 
``SatSuperframeConf0``.

\ 

.. _tab-superframe-conf:

.. table:: Superframe configuration attributes

	============================================================     ========================================================================
	Name of the attribute                                            Description
	============================================================     ========================================================================
	ns3::SatSuperframeConf0:: Frame0_AllocatedBandwidthHz            Allocated bandwidth for the frame. 
	ns3::SatSuperframeConf0:: Frame0_CarrierAllocatedBandwidthHz     Allocated bandwidth for each carrier in the frame.
	ns3::SatSuperframeConf0:: Frame0_CarrierRollOff                  Roll-off factor for each carrier in the frame.
	ns3::SatSuperframeConf0:: Frame0_CarrierSpacing                  Spacing for each carrier in the frame.
	ns3::SatSuperframeConf0:: Frame0_RandomAccessFrame               Defined if frame and its carriers are for random access or not (for DA).
        ns3::SatSuperframeConf0:: Frame0_LowerLayerService               LLS configuration to use for the frame.
	============================================================     ======================================================================== 
 
Sum of allocated bandwidths of used frames cannot exceed the calculated bandwidth 
for the return link channels, see `Return link frame configuration`_. ``Frame0_CarrierAllocatedBandwidthHz`` 
defines bandwidth for each carrier in the frame and also the number of the frames in use in the frame. 
Value of this attribute cannot exceed the value given for attribute ``Frame0_AllocatedBandwidthHz`` for
the frame. This ensures that there is at least one carrier always available in the frame. There is no
limitation for count of RA or DA frames in the superframe; nor for count of carriers in each frame. 

To better understand what are the implications of changing ``Frame0_LowerLayerService``, see
`LLS configuration`_.

Waveform configuration
######################

The waveform configuration has influence to time slot configuration of the superframes. 
Frames in supeframe are constructed with timeslots using the waveform defined by the 
attribute ns3:SatWaveformConf::DefaultWfId. The construction means that this timeslot 
specified duration of the frame (as many slot as fit in given target duration).

- When superframe configuration type 0 is in use, the timeslot constructed based 
  on attribute ``ns3:SatWaveformConf::DefaultWfId`` is always used for scheduling 
  timeslots for the UTs by ``SatBeamScheduler``. 
- When superframe configuration type 1 is in use the timeslot constructed based 
  on attribute ``ns3:SatWaveformConf::DefaultWfId`` specified duration for the scheduled 
  timeslots for the UTs. The waveform selection for the timeslot is based on C/N0 
  estimation (the possible). Anyway for control timeslots is always used the most 
  robust wave form.
- When superframe configuration type 2 is in use the timeslot constructed based 
  on attribute ``ns3:SatWaveformConf::DefaultWfId`` doesn’t have influence for 
  the scheduled timeslots for the UTs. The waveform selection for the timeslot 
  is based on C/N0 estimation (the possible) as for configuration 1. But in 
  addition to waveform also timeslot duration can change between short and long 
  waveforms. Again for control timeslots is always used the most robust wave form.
  
For configuration types 1 and 2 attribute ``ns3:SatWaveformConf::AcmEnabled`` 
is set as true (enabled). Otherwise behavior is same as with configuration 0. If C/N0 estimation 
is unknown then most robust waveform is used when configuration types 1 or 2 are used.


LLS configuration
#################

Lower Layer Service (LLS) can be configured currently to be used for Dedicated Access (DA) 
and Random Access (RA) services. Configuration is done by attributes of the 
``SatLowerLayerServiceConf`` class implementation LLS configuration. Attribute 
``ns3::SatLowerLayerServiceConf::DaServiceCount`` select how many of these four 
configurable DA services are in uses starting from service 0. The number of RA services 
to use are selected by attribute ``ns3::SatLowerLayerServiceConf::RaServiceCount`` 
(0 or 1 currently only selectable).

\ 

:ref:`tab-llserv` shortly introduces attributes 
affecting all used DA or RA services.

.. _tab-llserv:

.. table:: Lower layer service attributes

   ===================================================================         ==============================================================================================================================================================
   Name of the attribute                                                       Description
   ===================================================================         ==============================================================================================================================================================
   ns3::SatLowerLayerServiceConf:: DynamicRatePersistence                      Dynamic rate persistence count for the scheduling in case that capacity request is not received from UT.
   ns3::SatLowerLayerServiceConf:: VolumeBacklogPersistence                    Volume backlog persistence count for the scheduling in case that capacity request is not received from UT.
   ns3::SatLowerLayerServiceConf:: DefaultControlRandomizationInterval         Default value for the randomization interval to be used when selecting a Slotted ALOHA timeslot for the contention control burst, given in milliseconds.
   ns3::SatLowerLayerServiceConf:: RbdcQuantizationSmallStepKbps               Quantization interval for RBDC values in the smaller value range (below RbdcQuantizationThresholdKbps), given in kbps.
   ns3::SatLowerLayerServiceConf:: RbdcQuantizationLargeStepKbps               Quantization interval for RBDC values in the larger value range (above RbdcQuantizationThresholdKbps), given in kbps.
   ns3::SatLowerLayerServiceConf:: RbdcQuantizationThresholdKbps               RBDC quantization threshold in Kbps. If RBDC rate is lower, then RbdcQuantizationSmallStepKbps is used, if higher, then RbdcQuantizationLargeStepKbps is used.
   ns3::SatLowerLayerServiceConf:: VbdcQuantizationSmallStepKB                 Quantization interval for VBDC values in the smaller value range (below VbdcQuantizationThresholdKB), given in kbytes.
   ns3::SatLowerLayerServiceConf:: VbdcQuantizationLargeStepKB                 Quantization interval for VBDC values in the larger value range (above VbdcQuantizationThresholdKB), given in kbytes.
   ns3::SatLowerLayerServiceConf:: VbdcQuantizationThresholdKB                 VBDC quantization threshold in kbytes. If VBDC bytes are lower, then VbdcQuantizationSmallStepKB is used, if higher, then VbdcQuantizationLargeStepKB is used.
   ===================================================================         ==============================================================================================================================================================

:ref:`tab-llserv-da` introduces all DA service specific attributes. 
In the table DA service 0 is used as example, 
but configuration for other services has identical structure.

\ 

.. _tab-llserv-da:

.. table:: Lower layer service attributes for DA services

   =====================================================================     ====================================================================================
   Name of the attribute                                                     Description
   =====================================================================     ====================================================================================
   ns3::SatLowerLayerServiceConf:: DaService0_ConstantAssignmentProvided     Used enable or disable constant assignment. 
   ns3::SatLowerLayerServiceConf:: DaService0_RbdcAllowed                    Used enable or disable RBDC.
   ns3::SatLowerLayerServiceConf:: DaService0_VolumeAllowed                  Used enable or disable VBDC.
   ns3::SatLowerLayerServiceConf:: DaService0_ConstantServiceRate            Constant service rate assigned (kbps), if constant assignment is enabled.
   ns3::SatLowerLayerServiceConf:: DaService0_MaximumServiceRate             Maximum service rate (kbps).
   ns3::SatLowerLayerServiceConf:: DaService0_MinimumServiceRate             Minimum service rate (kbps).
   ns3::SatLowerLayerServiceConf:: DaService0_MaximumBacklogSize             Maximum backlog size (Kbytes).
   =====================================================================     ====================================================================================

:ref:`tab-llserv-ra` introduces all RA service 
specific attributes. Only one RA service is configurable currently. 

\ 


.. _tab-llserv-ra:

.. table:: Lower layer service attributes for RA services

	======================================================================================        ==========================================================================================================================================================
	Name of the attribute                                                                         Description
	======================================================================================        ==========================================================================================================================================================
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ MaximumUniquePayloadPerBlock                 Indicates the maximum number of unique payloads that the RCST is permitted to send in an RA block.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ MaximumConsecutiveBlockAccessed              Indicates the maximum number of consecutive RA blocks that the RCST is permitted to access for sending of unique payloads.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ MinimumIdleBlock                             Indicates the minimum number of RA blocks that the RCST shall ignore for a given RA allocation channel index after having accessed a maximum allowed number of consecutive RA blocks.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ BackOffTimeInMilliSeconds                    Indicates the time that a terminal shall wait before transmitting in the RA allocation channel. This parameter is for normal load state.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ HighLoadBackOffTimeInMilliSeconds            Indicates the time that a terminal shall wait before transmitting in the RA allocation channel. This parameter is for high load state.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ BackOffProbability                           Indicates the probability for entering in back off state. When not in back off state, this is also the probability that the terminal shall avoid accessing the RA allocation channel. This parameter is for normal load state.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ HighLoadBackOffProbability                   Indicates the probability for entering in back off state. When not in back off state, this is also the probability that the terminal shall avoid accessing the RA allocation channel. This parameter is for high load state.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ NumberOfInstances                            This field indicates the number of bursts to be transmitted for each unique payload. Value "1" indicates Slotted ALOHA operation. Values above "1" indicate CRDSA operation.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ AverageNormalizedOfferedLoadThreshold        Indicates the average normalized offered load threshold for dynamic load control. Dynamic load control moves to high load state and parameterization if the load exceeds this threshold.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ SlottedAlohaAllowed                          Indicates whether this RA service can be used to send Slotted Aloha traffic.
	ns3\:\:SatLowerLayerServiceConf\:\: RaService0\_ CrdsaAllowed                                 Indicates whether this RA service can be used to send CRDSA traffic.
	======================================================================================        ==========================================================================================================================================================


Link Budget configuration
##########################

Configuration of the link budget for the satellite module can be best studied from 
Link budget example found from ``/contrib/satellite/examples`` directory and implemented 
in file ``sat-link-budget-example.cc``. Parameters (attributes) affecting link budget are 
found in PHY objects. Every type of the PHY object has slight differences to other PHY objects 
related to link budget configuration. The example reads ``sat-link-budget-input-attributes.xml`` 
file as an input. In this file are all needed attributes for link budget with default values 
(same as set in code level), just changing needed values and re-executing is enough to simulate 
effects on link budget. ``SatGwPhy`` prefixed attributes are for PHY in GW, ``SatGeoFeederPhy`` 
prefixed attributes are for feeder link PHY in Geo Satellite, ``SatGeoUserPhy`` prefixed attributes 
are for user link PHY in Geo Satellite and ``SatUtPhy`` prefixed attributes are for PHY in UT.

Interference configuration
##########################

Used interference model are configured by attributes in Helpers.
Interference model for DA can configured per each link with the following by attributes. 
Interference attributes are presented in :ref:`tab-interference`.

\ 

.. _tab-interference:

.. table:: Interference attributes

	+---------------------------------------------+
	|Name of the attribute                        |
	+=============================================+
	|ns3::SatGeoHelper::DaFwdLinkInterferenceModel|
	+---------------------------------------------+
	|ns3::SatGeoHelper::DaRtnLinkInterferenceModel|
	+---------------------------------------------+
	|ns3::SatGwHelper::DaRtnLinkInterferenceModel |
	+---------------------------------------------+
	|ns3::SatUtHelper::DaFwdLinkInterferenceModel |
	+---------------------------------------------+

For random access interference can be configured system level (influence in return link only) with 
``ns3::SatBeamHelper::RaInterferenceModel`` attribute.
Possible model to configure are ``Constant``, ``Trace``, ``PerPacket`` (packet by packet), or
``PerFragment``. Difference between ``PerPacket`` and ``PerFragment`` is that, using ``PerPacket``,
each packet overlapping an other is counted as interfering on the whole other packet, whereas
``PerFragment`` take each overlapping fragment independently to compute interferences on each
of them; resulting in a vector of interference for each packet.

Interference elimination method can be configured for Random-Access packet decoding. Two method are
proposed:

 * Perfect cancelation: interferences from a decoded packet/fragment are removed from all other
   packets/fragment it is interfering with (default behaviour);
 * Residual interference elimination: interferences from a decoded packet/fragment leave residual
   power when its influence is removed from the other packets/fragments it is interfering with.

Interference elimination attributes are presented in :ref:`tab-interference-elimination`..

\

.. _tab-interference-elimination:

.. table:: Interference Elimination attributes

	+--------------------------------------------------+
	|Name of the attribute                             |
	+==================================================+
	|ns3::SatBeamHelper::RaInterferenceEliminationModel|
	+--------------------------------------------------+

BB Frame configuration
######################

Configuration for BB frames and BB frame configuration are done by class ``SatBbFrameConf`` attributes. 
These attributes can be seen from Doxygen documentation.

Forward link scheduler configuration
####################################

Configuration for forward link scheduler is done by class ``SatFwdLinkScheduler`` attributes.
These attributes can be seen from Doxygen documentation. Also BB frame configuration has direct 
influence to forward link scheduler functionality, see `BB Frame configuration`_. 

Return link scheduler configuration
####################################

Configuration for return link scheduler is done by class ``SatBeamScheduler`` and 
``SatFrameAllocator`` attributes. These attributes can be seen from Doxygen documentation. 
Also superframe, waveform and lower layer service configurations have influence to return 
link scheduling (``SatBeamScheduler``) functionality. See chapters 
`Superframe structure configuration`_, `Waveform configuration`_ and `LLS configuration`_.


Request manager configuration
#############################

The most important class to be configured for UT request manager is the lower layer service configuration, 
which is already presented in section `LLS configuration`_. It can be used to enable and disable CRA/VBDC/RBDC and change 
parameters for each RC index individually. 

The request manager evaluation interval may be changed by ``ns3::SatRequestManager::EvaluationInterval`` 
attribute.

ARQ configuration
##################

ARQ maybe enabled and disabled through SatUtHelper attributes ``ns3::SatUtHelper::EnableRtnLinkArq`` 
and ``ns3::SatUtHelper::EnableFwdLinkArq.`` The ARQ specific attributes are described in 
:ref:`tab-arq`. 

\ 

.. _tab-arq:

.. table:: ARQ configuration attributes


	===========================================================        ===========================================================================================================================================================
	Name of the attribute                                              Description
	===========================================================        ===========================================================================================================================================================
	ns3:: SatReturnLinkEncapsulator:: MaxRtnArqSegmentSize             Maximum size for the RTN link segment with ARQ. This is set by default to 38 bytes, so that the retransmissions would certainly fit into the time slot.
	ns3:: SatReturnLinkEncapsulator:: MaxNoOfRetransmissions           Maximum number of allowed retransmissions for the RTN link ARQ.
	ns3:: SatReturnLinkEncapsulator:: ReransmissionTimer               Time to wait for an ACK before sending a retransmission in RTN link ARQ.
	ns3:: SatReturnLinkEncapsulator:: WindowSize                       Number of simultaneous and consecutive processes allowed for RTN link ARQ.
	ns3:: SatReturnLinkEncapsulator:: ArqHeaderSize                    Header size for ARQ in RTN link.
	ns3:: SatReturnLinkEncapsulator:: RxWaitingTime                    Maximum waiting time at the receiver side before moving the window forward and accepting an error.
	ns3:: SatGenericStreamEncapsulator:: MaxNoOfRetransmissions        Maximum number of allowed retransmissions for the FWD link ARQ.
	ns3:: SatGenericStreamEncapsulator:: ReransmissionTimer            Time to wait for an ACK before sending a retransmission in RTN link ARQ.
	ns3:: SatGenericStreamEncapsulator:: WindowSize                    Number of simultaneous and consecutive processes allowed for RTN link ARQ.
	ns3:: SatGenericStreamEncapsulator:: ArqHeaderSize                 Header size for ARQ in RTN link.
	ns3:: SatGenericStreamEncapsulator:: RxWaitingTime                 Maximum waiting time at the receiver side before moving the window forward and accepting an error.
	===========================================================        ===========================================================================================================================================================



Examples
========

Example scripts are listed in tables :ref:`tab-ra-examples`, :ref:`tab-systest-examples`, 
:ref:`tab-trace-examples`, :ref:`tab-traffic-examples` and :ref:`tab-training-examples`.


.. _tab-ra-examples:

.. table:: Random access examples

	+--------------------------------------------------------------------------------------+ 
	| Example script                                                                       | 
	+======================================================================================+ 
	| sat-random-access-crdsa-collision-example.cc                                         | 
	+--------------------------------------------------------------------------------------+ 
	| sat-random-access-crdsa-example.cc                                                   | 
	+--------------------------------------------------------------------------------------+ 
	| sat-random-access-dynamic-load-control-example.cc                                    | 
	+--------------------------------------------------------------------------------------+ 
	| sat-random-access-example.cc                                                         | 
	+--------------------------------------------------------------------------------------+ 
	| sat-random-access-slotted-aloha-collision-example.cc                                 | 
	+--------------------------------------------------------------------------------------+ 
	| sat-random-access-slotted-aloha-example.cc                                           | 
	+--------------------------------------------------------------------------------------+ 
	| sat-ra-sim-tn9.cc                                                                    | 
	+--------------------------------------------------------------------------------------+ 
	| sat-ra-sim-tn9-comparison.cc                                                         | 
	+--------------------------------------------------------------------------------------+ 

\ 

.. _tab-systest-examples:

.. table:: System test examples

	+--------------------------------------------------------------------------------------+ 
	| Example script                                                                       | 
	+======================================================================================+ 
	| sat-fwd-system-test-example.cc                                                       | 
	+--------------------------------------------------------------------------------------+ 
	| sat-rtn-system-test-example.cc                                                       |
	+--------------------------------------------------------------------------------------+ 


\ 

.. _tab-trace-examples:

.. table:: Trace examples

	+--------------------------------------------------------------------------------------+ 
	| Example script                                                                       | 
	+======================================================================================+ 
	| sat-trace-input-external-fading-example.cc                                           | 
	+--------------------------------------------------------------------------------------+ 
	| sat-trace-input-fading-example.cc                                                    | 
	+--------------------------------------------------------------------------------------+ 
	| sat-trace-input-interference-example.cc                                              | 
	+--------------------------------------------------------------------------------------+ 
	| sat-trace-input-rx-power-example.cc                                                  | 
	+--------------------------------------------------------------------------------------+ 
	| sat-trace-output-example.cc                                                          | 
	+--------------------------------------------------------------------------------------+ 


\ 

.. _tab-traffic-examples:

.. table:: Traffic examples

	+--------------------------------------------------------------------------------------+ 
	| Example script                                                                       | 
	+======================================================================================+ 
	| sat-cbr-example.cc                                                                   | 
	+--------------------------------------------------------------------------------------+ 
	| sat-cbr-full-example.cc                                                              | 
	+--------------------------------------------------------------------------------------+ 
	| sat-cbr-stats-example.cc                                                             | 
	+--------------------------------------------------------------------------------------+ 
	| sat-cbr-user-defined-example.cc                                                      | 
	+--------------------------------------------------------------------------------------+ 
	| sat-dama-http-sim-tn9.cc                                                             | 
	+--------------------------------------------------------------------------------------+ 
	| sat-dama-onoff-sim-tn9.cc                                                            | 
	+--------------------------------------------------------------------------------------+ 
	| sat-cbr-stats-example.cc                                                             | 
	+--------------------------------------------------------------------------------------+ 
	| sat-http-example.cc                                                                  | 
	+--------------------------------------------------------------------------------------+ 
	| sat-nrtv-example.cc                                                                  | 
	+--------------------------------------------------------------------------------------+ 
	| sat-onoff-example.cc                                                                 | 
	+--------------------------------------------------------------------------------------+ 


\ 

.. _tab-training-examples:

.. table:: Training examples

	+--------------------------------------------------------------------------------------+ 
	| Example script                                                                       | 
	+======================================================================================+ 
	| sat-training-example.cc                                                              | 
	+--------------------------------------------------------------------------------------+ 
	| sat-tutorial-example.cc                                                              | 
	+--------------------------------------------------------------------------------------+ 





Troubleshooting
===============

- Examples run without data package installed will crash to a fatal error.

