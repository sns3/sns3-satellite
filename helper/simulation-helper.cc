/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Lauri Sormunen <lauri.sormunen@magister.fi>
 */

#include "simulation-helper.h"

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>
#include <ns3/singleton.h>
#include <ns3/enum.h>
#include <ns3/config.h>
#include <ns3/config-store.h>
#include <ns3/satellite-env-variables.h>
#include <ns3/advanced-on-off-application.h>
#include <ns3/packet-sink.h>
#include <ns3/random-variable-stream.h>
#include <ns3/satellite-enums.h>

NS_LOG_COMPONENT_DEFINE ("SimulationHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimulationHelper);
TypeId
SimulationHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SimulationHelper")
      .SetParent<Object> ()
      .AddConstructor<SimulationHelper> ()
      .AddAttribute ("SimTime",
                     "Simulation time",
                     TimeValue (Seconds (100)),
                     MakeTimeAccessor (&SimulationHelper::m_simTime),
                     MakeTimeChecker (Seconds (10)))
      .AddAttribute ("UiOutputPath",
                     "Path to store simulation information for the UI.",
                     StringValue (""),
                     MakeStringAccessor (&SimulationHelper::m_uiOutputPath),
                     MakeStringChecker ())
;
    return tid;
}

TypeId
SimulationHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SimulationHelper::SimulationHelper ()
  : m_utCount (0),
    m_utUserCount (0),
    m_numberOfConfiguredFrames (0),
    m_randomAccessConfigured (false)
{
  NS_FATAL_ERROR ("SimulationHelper: Default contructor not in use. Please create with simulation name. ");
}

SimulationHelper::SimulationHelper (std::string simulationName)
  : m_utCount (0),
    m_utUserCount (0),
    m_numberOfConfiguredFrames (0),
    m_randomAccessConfigured (false)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  if (m_uiOutputPath != "")
    {
      StringValue simulationTag;
      Singleton<SatEnvVariables>::Get ()->GetAttribute ("SimulationTag", simulationTag);
      m_uiOutputFileName = m_uiOutputPath + "/" + simulationTag.Get ();
      std::remove (m_uiOutputFileName.c_str ());
    }

  m_simulationName = simulationName;
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue (m_simulationName));
}

SimulationHelper::~SimulationHelper ()
{
  NS_LOG_FUNCTION (this);

}

void
SimulationHelper::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  if (m_uiOutputFileName != "")
    {
      Ptr<SatOutputFileStreamStringContainer> outputContainer = CreateObject<SatOutputFileStreamStringContainer> (m_uiOutputFileName.c_str (), std::ios::out);
      outputContainer->AddToContainer (Singleton<SatEnvVariables>::Get ()->GetOutputPath ());
      outputContainer->WriteContainerToFile ();
    }
}

void
SimulationHelper::SetUtCountPerBeam (uint32_t count)
{
  NS_LOG_FUNCTION (this << count);

  m_utCount = CreateObject<ConstantRandomVariable> ();
  m_utCount->SetAttribute("Constant", DoubleValue (count));
}

void
SimulationHelper::SetUtCountPerBeam (Ptr<RandomVariableStream> rs)
{
  NS_LOG_FUNCTION (this << &rs);

  m_utCount = rs;
}

void
SimulationHelper::SetUserCountPerUt (uint32_t count)
{
  NS_LOG_FUNCTION (this << count);

  m_utUserCount = CreateObject<ConstantRandomVariable> ();
  m_utUserCount->SetAttribute("Constant", DoubleValue (count));
}

void
SimulationHelper::SetSimulationTime (double seconds)
{
  NS_LOG_FUNCTION (this << seconds);

  m_simTime = Seconds (seconds);
}

void
SimulationHelper::SetOutputTag (std::string tag)
{
  NS_LOG_FUNCTION (this << tag);

  // Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (tag));
}

void
SimulationHelper::SetOutputPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->SetOutputPath (path);
}

void
SimulationHelper::SetDefaultValues ()
{
  NS_LOG_FUNCTION (this);

  DisableRandomAccess ();
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", StringValue ("Configuration_0"));
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
  Config::SetDefault ("ns3::SatSuperframeSeq::TargetDuration", TimeValue (MilliSeconds (100)));

  Config::SetDefault ("ns3::SatRequestManager::EvaluationInterval", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatRequestManager::EnableOnDemandEvaluation", BooleanValue (false));

  Config::SetDefault ("ns3::SatBbFrameConf::BBFrameUsageMode", StringValue ("NormalFrames"));

  ConfigureFrequencyBands ();
  ConfigureFrame (0, 0, 2.0e7, 2.0e6, 0.2, 0.3, false);

  SetErrorModel(SatPhyRxCarrierConf::EM_AVI);
  SetInterferenceModel (SatPhyRxCarrierConf::IF_PER_PACKET);

  // ACM enabled
  EnableAcm(SatEnums::LD_FORWARD);
  EnableAcm(SatEnums::LD_RETURN);

  DisableAllCapacityAssignmentCategories ();
  EnableOnlyVbdc (3);

  EnablePeriodicalControlSlots (MilliSeconds (100));

  Config::SetDefault ("ns3::SatUtHelper::EnableChannelEstimationError", BooleanValue (true));
  Config::SetDefault ("ns3::SatGwHelper::EnableChannelEstimationError", BooleanValue (true));

  Config::SetDefault ("ns3::SatGwMac::DummyFrameSendingEnabled", BooleanValue (false));

  Config::SetDefault ("ns3::SatQueue::MaxPackets", UintegerValue (10000));
}

void
SimulationHelper::DisableAllCapacityAssignmentCategories ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaServiceCount", UintegerValue (4));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
}

void
SimulationHelper::EnableOnlyConstantRate (uint32_t rcIndex, double rateKbps)
{
  NS_LOG_FUNCTION (this << rcIndex << rateKbps);

  std::stringstream ss;
  ss << rcIndex;
  std::string attributeDefault ("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

  Config::SetDefault (attributeDefault + "_ConstantAssignmentProvided", BooleanValue (true));
  Config::SetDefault (attributeDefault + "_RbdcAllowed", BooleanValue (false));
  Config::SetDefault (attributeDefault + "_VolumeAllowed", BooleanValue (false));
}

void
SimulationHelper::EnableOnlyRbdc (uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << rcIndex);

  std::stringstream ss;
  ss << rcIndex;
  std::string attributeDefault ("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

  Config::SetDefault (attributeDefault + "_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault (attributeDefault + "_RbdcAllowed", BooleanValue (true));
  Config::SetDefault (attributeDefault + "_VolumeAllowed", BooleanValue (false));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DynamicRatePersistence", UintegerValue (10));
}

void
SimulationHelper::EnableOnlyVbdc (uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << rcIndex);

  std::stringstream ss;
  ss << rcIndex;
  std::string attributeDefault ("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

  Config::SetDefault (attributeDefault + "_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault (attributeDefault + "_RbdcAllowed", BooleanValue (false));
  Config::SetDefault (attributeDefault + "_VolumeAllowed", BooleanValue (true));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::VolumeBacklogPersistence", UintegerValue (10));
}

void
SimulationHelper::EnableFca ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (true));
}

void
SimulationHelper::DisableFca ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (false));
}


void
SimulationHelper::EnablePeriodicalControlSlots (Time periodicity)
{
  NS_LOG_FUNCTION (this << periodicity.GetSeconds ());

  // Controls slots
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (periodicity));
}

void
SimulationHelper::EnableArq (SatEnums::SatLinkDir_t dir)
{
  NS_LOG_FUNCTION (this << dir);

  switch (dir)
   {
     case SatEnums::LD_FORWARD:
       {
         Config::SetDefault ("ns3::SatLlc::FwdLinkArqEnabled", BooleanValue (true));

         Config::SetDefault ("ns3::SatGenericStreamEncapsulatorArq::MaxNoOfRetransmissions", UintegerValue (2));
         Config::SetDefault ("ns3::SatGenericStreamEncapsulatorArq::RetransmissionTimer", TimeValue (MilliSeconds (600)));
         Config::SetDefault ("ns3::SatGenericStreamEncapsulatorArq::WindowSize", UintegerValue (10));
         Config::SetDefault ("ns3::SatGenericStreamEncapsulatorArq::ArqHeaderSize", UintegerValue (1));
         Config::SetDefault ("ns3::SatGenericStreamEncapsulatorArq::RxWaitingTime", TimeValue (Seconds (1.8)));
         break;
       }
     case SatEnums::LD_RETURN:
       {
         Config::SetDefault ("ns3::SatLlc::RtnLinkArqEnabled", BooleanValue (true));

         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::MaxRtnArqSegmentSize", UintegerValue (38));
         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::MaxNoOfRetransmissions", UintegerValue (2));
         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::RetransmissionTimer", TimeValue (MilliSeconds (600)));
         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::WindowSize", UintegerValue (10));
         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::ArqHeaderSize", UintegerValue (1));
         Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::RxWaitingTime", TimeValue (Seconds (1.8)));
         break;
       }
     default:
       {
         NS_FATAL_ERROR ("Unsupported SatLinkDir_t!");
         break;
       }
   }
}

void
SimulationHelper::DisableRandomAccess ()
{
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_CONSTANT));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_NOT_DEFINED));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
}

void
SimulationHelper::EnableSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessConfigured == true)
    {
      NS_FATAL_ERROR ("Random access already configured!");
    }

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (1));

  EnableRandomAccess ();

}

void
SimulationHelper::EnableCrdsa ()
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessConfigured == true)
    {
      NS_FATAL_ERROR ("Random access already configured!");
    }

  Config::SetDefault ("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
  EnableRandomAccess ();
}

void
SimulationHelper::EnableRandomAccess ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval", TimeValue (MilliSeconds (100)));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaServiceCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_MODEL_RCS2_SPECIFICATION));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (50));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.99));

  m_randomAccessConfigured = true;

  SimulationHelper::ConfigureFrame (0, 0, 2.0e7, 2.0e6, 0.2, 0.3, true);
}

void
SimulationHelper::SetIdealPhyParameterization()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (false));
  Config::SetDefault ("ns3::SatChannel::ForwardingMode", StringValue ("OnlyDestNode"));
  Config::SetDefault ("ns3::SatChannel::RxPowerCalculationMode", StringValue ("RxPowerCalculation"));
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", StringValue ("FadingOff"));
  Config::SetDefault ("ns3::SatUtHelper::EnableChannelEstimationError", BooleanValue (false));
  Config::SetDefault ("ns3::SatGwHelper::EnableChannelEstimationError", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", StringValue ("Constant"));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", StringValue ("RaCollisionNotDefined"));

  SetErrorModel(SatPhyRxCarrierConf::EM_NONE);
  SetInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT);
}


void
SimulationHelper::EnableAcm (SatEnums::SatLinkDir_t dir)
{
  NS_LOG_FUNCTION (this << dir);

  switch (dir)
  {
    case SatEnums::LD_FORWARD:
      {
        Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
        Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("QPSK_1_TO_2"));
        Config::SetDefault ("ns3::SatRequestManager::CnoReportInterval", TimeValue (Seconds (0.1)));
        Config::SetDefault ("ns3::SatFwdLinkScheduler::CnoEstimationMode", StringValue ("AverageValueInWindow"));
        Config::SetDefault ("ns3::SatFwdLinkScheduler::CnoEstimationWindow", TimeValue (Seconds (2)));
        break;
      }
    case SatEnums::LD_RETURN:
      {
        Config::SetDefault ("ns3::SatBeamScheduler::CnoEstimationMode", StringValue ("MinimumValueInWindow"));
        Config::SetDefault ("ns3::SatBeamScheduler::CnoEstimationWindow", TimeValue (Seconds (2)));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));
        Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (3));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported SatLinkDir_t!");
        break;
      }
  }
}

void
SimulationHelper::DisableAcm (SatEnums::SatLinkDir_t dir)
{
  NS_LOG_FUNCTION (this << dir);

  switch (dir)
  {
    case SatEnums::LD_FORWARD:
      {
        Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (false));
        Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("QPSK_1_TO_2"));
        Config::SetDefault ("ns3::SatRequestManager::CnoReportInterval", TimeValue (Seconds (100)));
        break;
      }
    case SatEnums::LD_RETURN:
      {
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (false));
        Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (3));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported SatLinkDir_t!");
        break;
      }
  }
}

void
SimulationHelper::ProgressCb ()
{
  std::cout << "Progress: " << Simulator::Now ().GetSeconds () << "/" << GetSimTime ().GetSeconds () << std::endl;
  Simulator::Schedule (Seconds (1), &SimulationHelper::ProgressCb, this);
}

void
SimulationHelper::CreateDefaultStats ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_satHelper != 0, "Satellite scenario not created yet!");

  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  statContainer = CreateObject<SatStatsHelperContainer> (m_satHelper);

  // Throughput
  statContainer->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  statContainer->AddAverageUtRtnPhyThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  statContainer->AddAverageBeamRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  statContainer->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  statContainer->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  statContainer->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  statContainer->AddGlobalRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddGlobalRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Granted resources
  statContainer->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);
  statContainer->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  statContainer->AddPerUtResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Frame load
  statContainer->AddGlobalFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // SINR
  statContainer->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  statContainer->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Delay
  statContainer->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  statContainer->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);

  // Packet error
  statContainer->AddGlobalRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Waveform
  statContainer->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Capacity request
  statContainer->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  statContainer->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
}

void
SimulationHelper::SetErrorModel (SatPhyRxCarrierConf::ErrorModel em, double errorRate)
{
  NS_LOG_FUNCTION (this << em << errorRate);

  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  if (errorRate == SatPhyRxCarrierConf::EM_CONSTANT)
    {
      Config::SetDefault ("ns3::SatPhyRxCarrierConf::ConstantErrorRatio", DoubleValue(errorRate));
    }
}

void
SimulationHelper::SetInterferenceModel(SatPhyRxCarrierConf::InterferenceModel ifModel,
                                       double constantIf)
{
  NS_LOG_FUNCTION (this << ifModel << constantIf);

  Config::SetDefault ("ns3::SatUtHelper::DaFwdLinkInterferenceModel", EnumValue (ifModel));
  Config::SetDefault ("ns3::SatGwHelper::DaRtnLinkInterferenceModel", EnumValue (ifModel));

  if (ifModel == SatPhyRxCarrierConf::IF_CONSTANT)
    {
      Config::SetDefault ("ns3::SatConstantInterference::ConstantInterferencePower", DoubleValue (constantIf));
    }
}

void
SimulationHelper::ConfigureFrame (uint32_t superFrameId,
                                  uint32_t frameId,
                                  double bw,
                                  double carrierBw,
                                  double rollOff,
                                  double carrierSpacing,
                                  bool isRandomAccess)
{
  NS_LOG_FUNCTION (this << frameId << bw << carrierBw << rollOff << carrierSpacing << isRandomAccess);

  std::stringstream sfId, fId;
  sfId << superFrameId;
  fId << frameId;
  std::string attributeDefault ("ns3::SatSuperframeConf" + sfId.str() + "::Frame" + fId.str());

  //Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType" value="ConfigType_0"/>
  Config::SetDefault (attributeDefault + "_AllocatedBandwidthHz", DoubleValue (bw));
  Config::SetDefault (attributeDefault + "_CarrierAllocatedBandwidthHz", DoubleValue (carrierBw));
  Config::SetDefault (attributeDefault + "_CarrierRollOff", DoubleValue (rollOff));
  Config::SetDefault (attributeDefault + "_CarrierSpacing", DoubleValue (carrierSpacing));
  Config::SetDefault (attributeDefault + "_RandomAccessFrame", BooleanValue (isRandomAccess));

  m_numberOfConfiguredFrames++;

  Config::SetDefault ("ns3::SatSuperframeConf" + sfId.str() + "::FrameCount",
                      UintegerValue (m_numberOfConfiguredFrames));

}

void
SimulationHelper::ConfigureFrequencyBands ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBaseFrequency", DoubleValue (2.75e+10));
  Config::SetDefault ("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue (5e+08));
  Config::SetDefault ("ns3::SatConf::FwdUserLinkBaseFrequency", DoubleValue (1.97e+10));

  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBaseFrequency", DoubleValue (1.77e+10));
  Config::SetDefault ("ns3::SatConf::RtnUserLinkBandwidth", DoubleValue (5e+08));
  Config::SetDefault ("ns3::SatConf::RtnUserLinkBaseFrequency", DoubleValue (2.95e+10));

  Config::SetDefault ("ns3::SatConf::UserLinkChannels", UintegerValue (4));
  Config::SetDefault ("ns3::SatConf::FeederLinkChannels", UintegerValue (16));

  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (1.25e+08));
  Config::SetDefault ("ns3::SatConf::FwdCarrierRollOff", DoubleValue (0.2));
  Config::SetDefault ("ns3::SatConf::FwdCarrierSpacing", DoubleValue (0.0));
}

void
SimulationHelper::EnableOutputTraces ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatChannel::EnableRxPowerOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatChannel::EnableFadingOutputTrace", BooleanValue (true));
}

void
SimulationHelper::ConfigureLinkBudget ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatGeoFeederPhy::RxTemperatureDbk", DoubleValue (28.4));
  Config::SetDefault ("ns3::SatGeoFeederPhy::RxMaxAntennaGainDb", DoubleValue (54));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxMaxAntennaGainDb", DoubleValue (54));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxMaxPowerDbw", DoubleValue (-4.38));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxOutputLossDb", DoubleValue (1.75));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxPointingLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxOboLossDb", DoubleValue (4));
  Config::SetDefault ("ns3::SatGeoFeederPhy::TxAntennaLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoFeederPhy::RxAntennaLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoFeederPhy::DefaultFadingValue", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoFeederPhy::ExtNoisePowerDensityDbwhz", DoubleValue (-207));
  Config::SetDefault ("ns3::SatGeoFeederPhy::ImIfCOverIDb", DoubleValue (27));
  Config::SetDefault ("ns3::SatGeoFeederPhy::FixedAmplificationGainDb", DoubleValue (82));
  Config::SetDefault ("ns3::SatGeoUserPhy::RxTemperatureDbk", DoubleValue (28.4));
  Config::SetDefault ("ns3::SatGeoUserPhy::RxMaxAntennaGainDb", DoubleValue (54));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxMaxAntennaGainDb", DoubleValue (54));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxMaxPowerDbw", DoubleValue (15));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxOutputLossDb", DoubleValue (2.85));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxPointingLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxOboLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatGeoUserPhy::TxAntennaLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoUserPhy::RxAntennaLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoUserPhy::DefaultFadingValue", DoubleValue (1));
  Config::SetDefault ("ns3::SatGeoUserPhy::OtherSysIfCOverIDb", DoubleValue (27.5));
  Config::SetDefault ("ns3::SatGeoUserPhy::AciIfCOverIDb", DoubleValue (17));
  Config::SetDefault ("ns3::SatGwPhy::RxTemperatureDbk", DoubleValue (24.62));
  Config::SetDefault ("ns3::SatGwPhy::RxMaxAntennaGainDb", DoubleValue (61.5));
  Config::SetDefault ("ns3::SatGwPhy::TxMaxAntennaGainDb", DoubleValue (65.2));
  Config::SetDefault ("ns3::SatGwPhy::TxMaxPowerDbw", DoubleValue (8.97));
  Config::SetDefault ("ns3::SatGwPhy::TxOutputLossDb", DoubleValue (2));
  Config::SetDefault ("ns3::SatGwPhy::TxPointingLossDb", DoubleValue (1.1));
  Config::SetDefault ("ns3::SatGwPhy::TxOboLossDb", DoubleValue (6));
  Config::SetDefault ("ns3::SatGwPhy::TxAntennaLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatGwPhy::RxAntennaLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatGwPhy::DefaultFadingValue", DoubleValue (1));
  Config::SetDefault ("ns3::SatGwPhy::ImIfCOverIDb", DoubleValue (22));
  Config::SetDefault ("ns3::SatGwPhy::AciIfWrtNoisePercent", DoubleValue (10));
  Config::SetDefault ("ns3::SatUtPhy::RxTemperatureDbk", DoubleValue (24.6));
  Config::SetDefault ("ns3::SatUtPhy::RxMaxAntennaGainDb", DoubleValue (44.6));
  Config::SetDefault ("ns3::SatUtPhy::TxMaxAntennaGainDb", DoubleValue (45.2));
  Config::SetDefault ("ns3::SatUtPhy::TxMaxPowerDbw", DoubleValue (4));
  Config::SetDefault ("ns3::SatUtPhy::TxOutputLossDb", DoubleValue (0.5));
  Config::SetDefault ("ns3::SatUtPhy::TxPointingLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatUtPhy::TxOboLossDb", DoubleValue (0.5));
  Config::SetDefault ("ns3::SatUtPhy::TxAntennaLossDb", DoubleValue (1));
  Config::SetDefault ("ns3::SatUtPhy::RxAntennaLossDb", DoubleValue (0));
  Config::SetDefault ("ns3::SatUtPhy::DefaultFadingValue", DoubleValue (1));
  Config::SetDefault ("ns3::SatUtPhy::OtherSysIfCOverIDb", DoubleValue (24.7));
}

void
SimulationHelper::EnableExternalFadingInputTrace ()
{
  NS_LOG_FUNCTION (this);

  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtInputMode", StringValue ("ListMode"));
  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
}

Ptr<SatHelper>
SimulationHelper::CreateSatScenario ()
{
  NS_LOG_FUNCTION (this);

  std::stringstream ss;
  ss << "Created scenario: " << std::endl;

  SatHelper::BeamUserInfoMap_t beamInfo;

  for (uint32_t i = 1; i <= 72; i++)
    {
      if (IsBeamEnabled (i))
        {
          SatBeamUserInfo info;
          uint32_t utCount = GetNextUtCount ();

          ss << "  Beam " << i << ": UT count= " << utCount;

          for (uint32_t j = 1; j < utCount + 1; j++)
            {
              uint32_t utUserCount = GetNextUtUserCount ();
              info.AppendUt (GetNextUtUserCount ());
              ss << ", " <<  j << ". UT user count= " << utUserCount;
            }

          beamInfo.insert (std::make_pair (i, info));

          ss << std::endl;
        }
    }

  std::string scenarioName = "Scenario72";
  m_satHelper = CreateObject<SatHelper> (scenarioName);
  m_satHelper->CreateUserDefinedScenario (beamInfo);

  NS_LOG_INFO (ss.str ());

  return m_satHelper;
}

void
SimulationHelper::SetBeams (std::string enabledBeams)
{
  NS_LOG_FUNCTION (this << enabledBeams);

  m_enabledBeamsStr = enabledBeams;
  const char * input = m_enabledBeamsStr.c_str ();
  std::string number;

  for (uint32_t i = 0; i <= m_enabledBeamsStr.size (); i++)
    {
      if (input[i] >= '0' && input[i] <= '9')
        {
          number.append (1, input[i]);
        }
      else
        {
          uint32_t beamId = std::atoi (number.c_str ());
          m_enabledBeams.insert (beamId);
          number = std::string ();
        }
    }
}

bool
SimulationHelper::IsBeamEnabled (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

  bool beamEnabled = false;

  if (m_enabledBeams.find (beamId) != m_enabledBeams.end ())
    {
      beamEnabled = true;
    }

  return beamEnabled;
}

void SimulationHelper::RunSimulation ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("--- " << m_simulationName << "---");
  //NS_LOG_INFO ("  Packet size in bytes: " << packetSizeMin << " - " << packetSizeMax);
  NS_LOG_INFO ("  Simulation length: " << m_simTime.GetSeconds ());
  NS_LOG_INFO ("  Enabled beams: " << m_enabledBeamsStr);
  //NS_LOG_INFO ("  Number of UTs: " << m_satHelper->GetGwUsers ().GetN());
  NS_LOG_INFO ("  Number of end users: " << m_satHelper->GetUtUsers ().GetN());
  NS_LOG_INFO ("  ");

  Simulator::Stop (m_simTime);
  Simulator::Run ();

  Simulator::Destroy ();
}

void
SimulationHelper::EnableProgressLogging ()
{
  NS_LOG_FUNCTION (this);

  Simulator::Schedule (Seconds (1), &SimulationHelper::ProgressCb, this);
}

void
SimulationHelper::ReadInputAttributesFromFile (std::string fileName)
{
  NS_LOG_FUNCTION (this << fileName);

  // To read attributes from file
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/" + fileName;
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
}

std::string
SimulationHelper::StoreAttributesToFile (std::string fileName)
{
  NS_LOG_FUNCTION (this);

  std::string outputPath;
  outputPath = Singleton<SatEnvVariables>::Get ()->GetOutputPath ();

  //  Store set attribute values to XML output file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (outputPath + "/" + fileName));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();

  return outputPath;
}

} // namespace ns3
