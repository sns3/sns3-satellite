+++++++++++++++++++++
Testing Documentation
+++++++++++++++++++++

Validation
**********

Test scripts are listed in table :ref:`tab-sat-tests`.

.. _tab-sat-tests:

.. table:: Satellite module tests
	
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Test                                      | Description                                                      | 
	+===========================================+==================================================================+ 
	| Satellite antenna pattern test            | This case creates the antenna gain patterns classes and          |
	|                                           | compares the antenna gain values and best beam ids for           |
	|                                           | the test positions.                                              |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite ARQ sequence number test        | ARQ sequence number handler test.                                |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite ARQ test                        | The test case generates several packets, which are buffered to   |
	|                                           | to RLE/GSE queue, and semi-random Tx opportunities to RLE/GSE.   |
	|                                           | The packets are received by the RLE/GSE                          |
	|                                           | receiver at a configured probability, and RLE/GSE receiver       |
	|                                           | generates ACKs, which also have a configurable error probability.|
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite channel estimation error test   | Test cases for FWD and RTN link channel estimation error.        | 
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite C/NO estimator test             | Test cases to unit test Satellite C/N0 estimator.                |      
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite control message container test  | Test cases to unit test satellite control message container.     | 
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite CRA test                        | This case tests successful transmission of UDP packets           |
	|                                           | from UT connected user to GW connected user in simple            |
	|                                           | scenario and using CRA only.                                     |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite fading external input trace test| Test case to unit test satellite fading external input trace     |
	|                                           | and container for these objects.                                 |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite free space loss test            | This case tests that SatFreeSpaceLoss object can be created      |
	|                                           | successfully and free space loss (FSL) is calculated correctly.  |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite geocoordinate test              | Test case to unit test that GeoCoordinate can be created with    |
	|                                           | valid values.                                                    |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite interference test               | This case tests that SatConstantInterference object can be       |
	|                                           | created successfully and interference value set is correct.      |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite link results test               | Test case for comparing a BLER value computed by                 |
	|                                           | DVB-RCS2 link results with a BLER value taken                    |
	|                                           | from a reference.                                                |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite mobility observer test          |                                                                  |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite mobility test                   | Test case to unit test satellite mobility's position             |
	|                                           | setting from random box position allocator.                      |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite Per-packet interference test    | System test cases for Satellite Per-Packet Interference Model.   |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite performance memory test         | This test case is expected to be run regular basis               |
	|                                           | and results saved                                                |
	|                                           | for tracking and analysing purposes. Valgrind                    |
	|                                           | program is utilized in test to validate memory usage.            |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite periodic control message test   | This case tests successful transmission of UDP packets from      |  
	|                                           | UT connected user to GW connected user in simple scenario        |
	|                                           | and using periodic control slots and VBDC only.                  |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite Random Access test              | Various random access test cases.                                |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite request manager test            | Test cases to test the UT request manager.                       |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite RLE test                        | Return Link Encapsulation test cases.                            |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite scenario creation               | Various satellite scenario creation test cases                   |
	|                                           | (simple, larger, full).                                          |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite simple unicast                  | Various point-to-point packet sending test cases.                |
	+-------------------------------------------+------------------------------------------------------------------+ 
	| Satellite waveform configuration test     | Test case to unit test the waveform configuration table for      |
	|                                           | DVB-RCS2                                                         |
	+-------------------------------------------+------------------------------------------------------------------+ 

