/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Minsu Kim <minsu1.kim@ryerson.ca>
 *
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"

#include <iostream>
//#include "gnuplot-iostream.h"
//#include <fstream>

using namespace ns3;


double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;
bool burst_mode;

Ptr<PacketSink> global_sink;
uint64_t global_lastTotalRx = 0;

QueueDiscContainer queueDiscs;

void PrintCurrentQueueSize() {	
	std::cout << queueDiscs.Get(0)->GetNPackets() << std::endl;
	Simulator::Schedule (MilliSeconds (1), &PrintCurrentQueueSize);
}

// Calculate Throughput (Byte per second)
void CalculateThroughput () {
	Time now = Simulator::Now ();
	double cur = (global_sink->GetTotalRx () - global_lastTotalRx);
	std::cout << now.GetSeconds() << "s: \t" << cur << "Byte/s" << std::endl;
	global_lastTotalRx = global_sink->GetTotalRx ();
	//Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
	Simulator::Schedule (Seconds (1), &CalculateThroughput);

}


// Trace sojourn time in queue
void SojournTimeTrace (Time sojournTime) {
	std::cout << "Sojourn time " << sojournTime.ToDouble (Time::MS) << "ms" << std::endl;
}

// Configure Application on TCP or UDP
void ConfigureApplication (uint16_t port, std::string socket_factory, Ptr<Node> src_node, Ptr<Node> sink_node, uint32_t active_time, uint32_t sleep_time, uint32_t mean_data_rate, uint32_t peak_mean_rate, uint32_t avg_pkt_size, Ipv4Address dst_interface) {


	/*uint32_t active_time_1 = 1;
	uint32_t active_time_2 = 2;
	uint32_t active_time_3 = 8;
	uint32_t active_time_4 = 25;
	uint32_t active_time_5 = 34;

	uint32_t sleep_time_1 = 4;
	uint32_t sleep_time_2 = 66;
	uint32_t sleep_time_3 = 241;
	uint32_t sleep_time_4 = 7985;
	uint32_t sleep_time_5 = 24832;*/
	

	OnOffHelper clientHelper (socket_factory, Address ());

	if (active_time == 1) {	
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	}
	else if (active_time == 2) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
	}
	else if (active_time == 3) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=8]"));
	}
	else if (active_time == 4) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=25]"));
	}
	else if (active_time == 5) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=34]"));
	}	


	if (sleep_time == 1) {	
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=4]"));
	}
	else if (sleep_time == 2) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=66]"));
	}
	else if (sleep_time == 3) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=241]"));
	}
	else if (sleep_time == 4) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=7985]"));
	}
	else if (sleep_time == 5) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=24832]"));
	}	
	else if (sleep_time == 0) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	}

	/*if (active_time == 1) {	
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::NormalRandomVariable[Mean=1.0|Variance=0.1|Bound=0.1]"));
	}
	else if (active_time == 2) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::NormalRandomVariable[Mean=2.0|Variance=0.1|Bound=0.1]"));
	}
	else if (active_time == 3) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::NormalRandomVariable[Mean=8.0|Variance=0.1|Bound=0.1]"));
	}
	else if (active_time == 4) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::NormalRandomVariable[Mean=25.0|Variance=0.1|Bound=0.1]"));
	}
	else if (active_time == 5) {
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::NormalRandomVariable[Mean=34.0|Variance=0.1|Bound=0.1]"));
	}	


	if (sleep_time == 1) {	
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=4.0|Variance=0.1|Bound=0.1]"));
	}
	else if (sleep_time == 2) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=66.0|Variance=0.1|Bound=0.1]"));
	}
	else if (sleep_time == 3) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=241.0|Variance=0.1|Bound=0.1]"));
	}
	else if (sleep_time == 4) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=7985.0|Variance=0.1|Bound=0.1]"));
	}
	else if (sleep_time == 5) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=24832.0|Variance=0.1|Bound=0.1]"));
	}
	else if (sleep_time == 0) {
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	}	*/
	if (burst_mode == true) {
		std::string burst_data_rate = std::to_string(mean_data_rate * peak_mean_rate) + "Bps";
		clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (burst_data_rate)));
	}
	else {
		std::string data_rate = std::to_string(mean_data_rate) + "Bps";
		clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (data_rate)));
	}

	clientHelper.SetAttribute ("PacketSize", UintegerValue (avg_pkt_size));

	// To differentiate each applications starting time
	double min = 0.0;
	double max = 3.0;
	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
	x->SetAttribute ("Min", DoubleValue (min));
	x->SetAttribute ("Max", DoubleValue (max));
	double differ_value = x->GetValue ();

	ApplicationContainer clientApp;
	AddressValue remoteAddress (InetSocketAddress (dst_interface, port));
	clientHelper.SetAttribute ("Remote", remoteAddress);
	clientApp.Add (clientHelper.Install (src_node));
	clientApp.Start (Seconds (client_start_time + differ_value));
	clientApp.Stop (Seconds (client_stop_time));

	if (socket_factory.compare("ns3::TcpSocketFactory") == 0) {
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sinkHelper.Install (sink_node);
    sinkApps.Start (Seconds (sink_start_time));
    sinkApps.Stop (Seconds (sink_stop_time));
  }

}



int main (int argc, char *argv[]) {

  uint32_t mean_rate_1 = 462;	// Bps
  uint32_t mean_rate_2 = 2461;
  uint32_t mean_rate_3 = 11388;
  //uint32_t mean_rate_4 = 42493;
  //uint32_t mean_rate_5 = 516540;
	
  uint32_t peak_mean_rate_1 = 11;
  uint32_t peak_mean_rate_2 = 66;
  //uint32_t peak_mean_rate_3 = 229;
  //uint32_t peak_mean_rate_4 = 474;
  //uint32_t peak_mean_rate_5 = 1253;

  uint32_t avg_pktSize_1 = 94;
  uint32_t avg_pktSize_2 = 144;
  uint32_t avg_pktSize_3 = 234;
  uint32_t avg_pktSize_4 = 327;
  uint32_t avg_pktSize_5 = 699;


	global_start_time = 0.0;
	global_stop_time = 10;

	burst_mode = true;

	std::string p2p_data_rate = "1Mbps";
	std::string csma_data_rate = "100Mbps";

	std::string p2p_delay = "0ms";
	std::string csma_delay = "0ms";

	/* DQN queuing discipline parameters*/
	uint32_t n_iot_group = 1;	// number of groups of IoT devices
	uint32_t n_non_iot_group = 1;
	uint32_t episode = 1;
	bool training_mode = true;
	uint32_t dq_threshold = 2000;
	bool status_trigger = false;
	double rewards_weight = 0.5;
	double update_period = 0.001;

	// Queue parameters
	/*uint32_t maxPackets = 300;
	uint32_t queueDiscLimitPackets = 1000;
	double minTh = 5;	// RED parameters
	double maxTh = 15;*/

	std::string queueDiscType = "PfifoFast";


  CommandLine cmd;
  cmd.AddValue ("queueDiscType", "Set Queue Discipline Type", queueDiscType);
  cmd.AddValue ("n_iot_group", "Set number of groups for IoT devices", n_iot_group);
  cmd.AddValue ("n_non_iot_group", "Set number of groups for Non-IoT devices", n_non_iot_group);
  cmd.AddValue ("simulationTime", "Set simulation time (seconds)", global_stop_time);
	cmd.AddValue ("episode", "Set N th episode for deep q learning", episode);  
	cmd.AddValue ("data_rate", "Data rate for link to cloud gateway", p2p_data_rate);
	cmd.AddValue ("delay", "Delay of link to cloud gateway", p2p_delay);
	cmd.AddValue ("burst_mode", "Set burst mode in the simulation", burst_mode);
	cmd.AddValue ("training_mode", "Set if it is training mode or testing mode", training_mode);
	cmd.AddValue ("dq_threshold", "Threshold for dequeue rate", dq_threshold);
	cmd.AddValue ("status_trigger", "Show status of queue in std output", status_trigger);
	cmd.AddValue ("rewards_weight", "Set weight for trade-off between delay and drop-rate", rewards_weight);
	cmd.AddValue ("update_period", "Set period for selecting an action", update_period);
	//cmd.AddValue ("queueLimitLength", "Set limited length of queue (packets)", queueDiscLimitPackets);

	cmd.Parse(argc, argv);

	// Set Simulation Time
	sink_start_time = global_start_time;
	sink_stop_time = global_stop_time + 0.1;
	client_start_time = sink_start_time + 0.1;
	client_stop_time = global_stop_time - 0.1;

	// Set TCP Congestion Algorithm
	//Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpBic"));
	Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", StringValue ("ns3::TcpPrrRecovery"));
	

	// Create Nodes
  //NS_LOG_INFO ("Create nodes");
  NodeContainer nodes;
  nodes.Create(1+(2*n_non_iot_group)+(21*n_iot_group) + 2 + 1);	// 2 -> number of cloud servers (datacenter, storage), 1 -> cloud gateway

	uint32_t last_node = 2*n_non_iot_group + 21*n_iot_group + 2 + 1;

  Names::Add ("edge_device", nodes.Get (last_node));    // AP
	
	/*
		IoT devices List

		<Hubs>
		1. Smart Things
  	2. Amazon Echo

		<Cameras>
  	3. Netatmo Welcome
  	4. TP-Link Day Night Cloud camera
  	5. Samsung SmartCam
  	6. Dropcam
  	7. Insteon Camera
  	8. Withings Smart Baby Monitor

		<Switches & Triggers>
    9. Belkin Wemo switch
  	10. TP-Link Smart plug
  	11. iHome
  	12. Belkin wemo motion sensor

		<Air quality sensors>
  	13. NEST Protect smoke alarm
  	14. Netatmo weather station

		<Healthcare devices>
  	15. Withings Smart scale
  	16. Blipcare Blood Pressure meter
  	17. Withings Aura smart sleep sensor

		<Light Bulbs>
  	18. LiFX Smart Bulb

		<Electronics>
  	19. Triby Speaker
  	20. PIX-STAR Photo frame
  	21. HP Printer

		Non-IoT devices List
		1. Laptop
		2. Smart phone
	*/

	NodeContainer p2pNodes;
	p2pNodes = NodeContainer (nodes.Get(last_node), nodes.Get(last_node - 1));
	
	std::string node_name;
	
  NodeContainer wifiApNode = nodes.Get(last_node);
	
  NodeContainer wifiStaNodes;
	for (uint32_t i = 0; i < 21*n_iot_group; i++) {
		node_name = "iot_wifiStaNode(" + std::to_string(i) + ")";
		
		wifiStaNodes.Add(nodes.Get(i));
  	Names::Add (node_name, nodes.Get (i));
	}
	for (uint32_t i = 0; i < 2 * n_non_iot_group; i++) {
		node_name = "non_iot_wifiStaNode(" + std::to_string(i) + ")";
		
		wifiStaNodes.Add(nodes.Get((21*n_iot_group) + i));
  	Names::Add (node_name, nodes.Get ((21*n_iot_group) + i));
	}
	
	NodeContainer csmaNodes;
	csmaNodes.Add (nodes.Get(last_node - 2));	// 0 -> datacenter	
	csmaNodes.Add (nodes.Get(last_node - 3));	// 1 -> storage
	csmaNodes.Add (nodes.Get(last_node - 1));	// 2 -> cloud gateway

	/*for (uint32_t i = 0; i < 2; i++) {
		node_name = "iot_csmaNode(" + std::to_string(i) + ")";
		
		csmaNodes.Add (nodes.Get(i + (21*n_iot_group) + (2*n_non_iot_group)));
  	Names::Add (node_name, nodes.Get ((21*n_iot_group) + (2*n_non_iot_group) + i));
	}*/
		
	/*node_name = "non_iot_csmaNode(" + std::to_string(2) + ")";
	csmaNodes.Add (nodes.Get((21*n_iot_group) + (2*n_non_iot_group) + 2));
  Names::Add (node_name, nodes.Get ((21*n_iot_group) + (2*n_non_iot_group) + 2));*/
	

	// Set Channels
	// p2p Channel	
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue (p2p_data_rate));
	p2p.SetChannelAttribute ("Delay", StringValue (p2p_delay));

	NetDeviceContainer p2pDevices;
	p2pDevices = p2p.Install (p2pNodes);
	
	// csma Channel
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue (csma_data_rate));
	// Set the speed-of-light delay of the channel to 6560 nano-seconds (arbitrarily chosen as 1 nanosecond per foot over a 100 meter segment)
	csma.SetChannelAttribute ("Delay", StringValue (csma_delay));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (csmaNodes);


	// Wifi Channel
  /*YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();

  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifiHelper;
  wifiHelper.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper wifiMac;*/

	WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency"    , DoubleValue (5e9));

	/* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
  wifiPhy.Set ("TxGain", DoubleValue (0));
  wifiPhy.Set ("RxGain", DoubleValue (0));
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue ("HtMcs7"),
                                     "ControlMode", StringValue ("HtMcs0"));

	/* Configure AP */
  Ssid ssid = Ssid ("network");
  wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "BeaconGeneration", BooleanValue (true));
  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, wifiApNode);
  
	/* Configure STA */
	wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices;
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, wifiStaNodes);


	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

	double base_location = (21*n_iot_group + 2*n_non_iot_group);
	
	const double pi = 3.1415926535897;
	double slice = 2 * pi / base_location;

	for (int i = 0; i < (int)base_location; i++) {
		double angle = slice * i;
		double x = 0 + 30 * std::cos(angle);
		double y = 0 + 30 * std::sin(angle);

		positionAlloc->Add (Vector (x, y, 0.0));
	}
	mobility.SetPositionAllocator (positionAlloc);

	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (wifiStaNodes);

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY", DoubleValue (0.0), "GridWidth", UintegerValue (base_location), "LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (wifiApNode);

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (-20.0), "MinY", DoubleValue (80.0), "DeltaX", DoubleValue (40.0), "DeltaY", DoubleValue (0.0), "GridWidth", UintegerValue (base_location), "LayoutType", StringValue ("RowFirst"));
	//mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (csmaNodes);	

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY", DoubleValue (50.0), "GridWidth", UintegerValue (base_location), "LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (csmaNodes.Get(2));

	// Install Internet Stacks
  InternetStackHelper stack;
	/*stack.Install (p2pNodes);
	stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);*/
	stack.InstallAll();


	// Set Queueing Discipline
	//Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue (std::to_string (maxPackets) + "p"));

	TrafficControlHelper tch;
	
	if (queueDiscType == "PfifoFast") {
		tch.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");

	}
	else if (queueDiscType == "RED") {
		tch.SetRootQueueDisc ("ns3::RedQueueDisc");
		
		/*Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
		Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
		Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));*/
	}
	else if (queueDiscType == "ARED") {
		tch.SetRootQueueDisc ("ns3::RedQueueDisc");
		
		/*Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
		Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
		Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
		Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));
		Config::SetDefault ("ns3::RedQueueDisc::LInterm", DoubleValue (50.0));*/
	}
	else if (queueDiscType == "CoDel") {
		tch.SetRootQueueDisc ("ns3::CoDelQueueDisc");
		
		//Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
	}
	else if (queueDiscType == "FqCoDel") {
		tch.SetRootQueueDisc ("ns3::FqCoDelQueueDisc");
		
		//Config::SetDefault ("ns3::FqCoDelQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
	}
	else if (queueDiscType == "PIE") {
		tch.SetRootQueueDisc ("ns3::PieQueueDisc");
		
		Config::SetDefault ("ns3::PieQueueDisc::MeanPktSize", UintegerValue(350));
		Config::SetDefault ("ns3::PieQueueDisc::DequeueThreshold", UintegerValue(3500));
	}

	else if (queueDiscType == "ACE") {
  	tch.SetRootQueueDisc ("ns3::AceQueueDisc");
    //csma_data_rate
    //Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
  } 
  else if (queueDiscType == "PGQ") {
    tch.SetRootQueueDisc ("ns3::PgqQueueDisc");
    //csma_data_rate
    //Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
  }
	else if (queueDiscType == "DQN") {
    tch.SetRootQueueDisc ("ns3::DqnQueueDisc");
    Config::SetDefault ("ns3::DqnQueueDisc::Episode", UintegerValue (episode));
    Config::SetDefault ("ns3::DqnQueueDisc::PhysicalDataRate", DataRateValue (p2p_data_rate));
		uint32_t dqn_rtt = std::stoi(p2p_delay.substr(0, p2p_delay.size()-2)) * 2;
    Config::SetDefault ("ns3::DqnQueueDisc::BaselineRTT", UintegerValue (dqn_rtt));
    Config::SetDefault ("ns3::DqnQueueDisc::TrainingMode", BooleanValue (training_mode));
    Config::SetDefault ("ns3::DqnQueueDisc::IotGroup", UintegerValue (n_iot_group));
		Config::SetDefault ("ns3::DqnQueueDisc::DequeueThreshold", UintegerValue (dq_threshold));
		Config::SetDefault ("ns3::DqnQueueDisc::StatusTrigger", BooleanValue (status_trigger));
    Config::SetDefault ("ns3::DqnQueueDisc::RewardsWeight", DoubleValue (rewards_weight));
    Config::SetDefault ("ns3::DqnQueueDisc::UpdatePeriod", TimeValue (Seconds(update_period)));
  }
	else {
		NS_ABORT_MSG ("--queueDiscType not valid");
	}

	//note: p2pDevices <- p2p.Install (p2pNodes); (Get(0): edge node, Get(1): cloud gateway)
	queueDiscs = tch.Install (p2pDevices.Get(0));

	//std::ostringstream oss;
	//oss << "/NodeList/22/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime";	

	//Config::ConnectWithoutContext (oss.str(), MakeCallback (&SojournTimeTrace));



	// Set IP Addresses
  Ipv4AddressHelper address;
  
	address.SetBase("163.152.0.0", "255.255.0.0");
	Ipv4InterfaceContainer p2pInterfaces;
	p2pInterfaces = address.Assign(p2pDevices);

	Ipv4InterfaceContainer csmaInterfaces;
	csmaInterfaces = address.Assign(csmaDevices);
	
	address.SetBase("10.16.0.0", "255.255.0.0");
	Ipv4InterfaceContainer staInterfaces;
	staInterfaces = address.Assign(staDevices);
	Ipv4InterfaceContainer apInterfaces;
	apInterfaces = address.Assign(apDevice);


	// Set up the routing
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	

	// Parameters for configuring application	
	// All traffics are going to the Internet or Cloud server.	
	// port, socket_factory, src_node, sink_node, active_time, sleep_time, mean_data_rate, avg_pkt_size, dst_interface
	
	for (uint32_t i = 0; i < 21*n_iot_group; i = i+21) {
		// smartThings (Hub) connection
		ConfigureApplication (i+10000, "ns3::TcpSocketFactory", wifiStaNodes.Get(i), csmaNodes.Get(0), 1, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (0));

		// Amazon Echo (Hub)
		ConfigureApplication (i+10001, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+1), csmaNodes.Get(0), 1, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_2, csmaInterfaces.GetAddress (0));
		
		// Netatmo Welcome (Camera)
		ConfigureApplication (i+10002, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+2), csmaNodes.Get(1), 1, 1, mean_rate_2, peak_mean_rate_2, avg_pktSize_4, csmaInterfaces.GetAddress (1));
		
		// TP-Link Day Night Cloud camera (Camera)
		ConfigureApplication (i+10003, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+3), csmaNodes.Get(1), 2, 2, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (1));
		
		// Samsung SmartCam (Camera)
		ConfigureApplication (i+10004, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+4), csmaNodes.Get(1), 4, 1, mean_rate_2, peak_mean_rate_1, avg_pktSize_4, csmaInterfaces.GetAddress (1));

		// Dropcam (Camera)
		ConfigureApplication (i+10005, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+5), csmaNodes.Get(1), 1, 1, mean_rate_1, peak_mean_rate_2, avg_pktSize_2, csmaInterfaces.GetAddress (1));

		// Insteon Camera (Camera)
		ConfigureApplication (i+10006, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+6), csmaNodes.Get(1), 3, 2, mean_rate_1, peak_mean_rate_1, avg_pktSize_2, csmaInterfaces.GetAddress (1));

		// Withings Smart Baby Monitor (Camera)
		ConfigureApplication (i+10007, "ns3::UdpSocketFactory", wifiStaNodes.Get(i+7), csmaNodes.Get(1), 1, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (1));

		// Belkin Wemo switch (Switch & Trigger)
		ConfigureApplication (i+10008, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+8), csmaNodes.Get(0), 5, 1, mean_rate_3, peak_mean_rate_1, avg_pktSize_4, csmaInterfaces.GetAddress (0));
		
		// TP-Link Smart plug (Switch & Trigger)
		ConfigureApplication (i+10009, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+9), csmaNodes.Get(0), 2, 3, mean_rate_1, peak_mean_rate_1, avg_pktSize_2, csmaInterfaces.GetAddress (0));

		// iHome (Switch & Trigger)
		ConfigureApplication (i+10010, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+10), csmaNodes.Get(0), 2, 2, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (0));

		// Belkin wemo motion sensor (Switch & Trigger)
		ConfigureApplication (i+10011, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+11), csmaNodes.Get(0), 2, 1, mean_rate_3, peak_mean_rate_1, avg_pktSize_3, csmaInterfaces.GetAddress (0));

		// NEST Protect smoke alarm (Air quality sensor)
		ConfigureApplication (i+10012, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+12), csmaNodes.Get(0), 5, 1, mean_rate_2, peak_mean_rate_1, avg_pktSize_3, csmaInterfaces.GetAddress (0));
		
		// Netatmo weather station (Air quality sensor)
		ConfigureApplication (i+10013, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+13), csmaNodes.Get(0), 2, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_3, csmaInterfaces.GetAddress (0));

		// Withings Smart scale (Healthcare device)
		ConfigureApplication (i+10014, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+14), csmaNodes.Get(0), 3, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_3, csmaInterfaces.GetAddress (0));
		
		// Blipcare Blood Pressure meter (Healthcare device)
		ConfigureApplication (i+10015, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+15), csmaNodes.Get(0), 4, 5, mean_rate_1, peak_mean_rate_1, avg_pktSize_2, csmaInterfaces.GetAddress (0));

		// Withings Aura smart sleep sensor (Healthcare device)
		ConfigureApplication (i+10016, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+16), csmaNodes.Get(0), 2, 1, mean_rate_1, peak_mean_rate_2, avg_pktSize_2, csmaInterfaces.GetAddress (0));

		// LiFX Smart Bulb (Light bulbs)
		ConfigureApplication (i+10017, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+17), csmaNodes.Get(0), 1, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (0));

		// Triby Speaker (Electronics)
		ConfigureApplication (i+10018, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+18), csmaNodes.Get(0), 1, 1, mean_rate_1, peak_mean_rate_2, avg_pktSize_2, csmaInterfaces.GetAddress (0));
		
		// PIX-STAR Photo frame (Electronics)
		ConfigureApplication (i+10019, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+19), csmaNodes.Get(0), 2, 1, mean_rate_1, peak_mean_rate_1, avg_pktSize_3, csmaInterfaces.GetAddress (0));

		// HP Printer (Electronics)
		ConfigureApplication (i+10020, "ns3::TcpSocketFactory", wifiStaNodes.Get(i+20), csmaNodes.Get(0), 1, 2, mean_rate_1, peak_mean_rate_1, avg_pktSize_1, csmaInterfaces.GetAddress (0));
	}

	for (uint32_t i = 0; i < 2 * n_non_iot_group; i = i+2) {
		// Laptop (Non-IoT device)
		ConfigureApplication (i+50001, "ns3::UdpSocketFactory", wifiStaNodes.Get((21*n_iot_group) + i), csmaNodes.Get(1), 2, 1, mean_rate_3, peak_mean_rate_2, avg_pktSize_5, csmaInterfaces.GetAddress (1));
	
		// Smart Phone (Non-IoT device)
		ConfigureApplication (i+50002, "ns3::UdpSocketFactory", wifiStaNodes.Get((21*n_iot_group) + i+1), csmaNodes.Get(1), 1, 1, mean_rate_2, peak_mean_rate_2, avg_pktSize_4, csmaInterfaces.GetAddress (1));
	}

	/*
	//Gnuplot parameters
	std::string fileNameWithNoExtension = "FlowVSThroughput";
	std::string graphicsFileName        = fileNameWithNoExtension + ".png";
	std::string plotFileName            = fileNameWithNoExtension + ".plt";
	std::string plotTitle               = "Flow vs Throughput";
	std::string dataTitle               = "Throughput";

	// Instantiate the plot and set its title.
	Gnuplot gnuplot (graphicsFileName);
	gnuplot.SetTitle (plotTitle);

	// Make the graphics file, which the plot file will be when it
	// is used with Gnuplot, be a PNG file.
	gnuplot.SetTerminal ("png");

	// Set the labels for each axis.
	gnuplot.SetLegend ("Flow", "Throughput");


	Gnuplot2dDataset dataset;
	dataset.SetTitle (dataTitle);
	dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);	
	*/

	// Flow monitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	//Simulator::Schedule (Seconds (1), &CalculateThroughput);
	//Simulator::Schedule (MilliSeconds (1), &PrintCurrentQueueSize);
	
	Simulator::Stop (Seconds (global_stop_time));
	
	//p2p.EnablePcapAll ("myTest-p2p");
	//phy.EnablePcap ("iot-topology-" + queueDiscType + "-" + std::to_string((int)round(global_stop_time)) + "s-phy", apDevice.Get(0));

	//AnimationInterface anim ("iot-topology-" + queueDiscType + "-" + std::to_string((int)round(global_stop_time)) + "s-animation.xml");
	//anim.SetConstantPosition (nodes.Get (21*n_iot_group+1), std::sqrt(21*nGroup) + 2, std::sqrt(21*nGroup) + 2);

	
	std::cout << "Running the simulation..." << std::endl;
	Simulator::Run();

	monitor->CheckForLostPackets();

	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

	double single_throughput = 0;
	double total_throughput = 0;
	double total_delay_sum = 0;
	double total_rx_packet = 0;
	double total_jitter_sum = 0;

	std::cout << "Simulation Time: " << global_stop_time << "s / P2P data rate: " << p2p_data_rate << " / P2P delay: " << p2p_delay << " / N of IoT group: " << n_iot_group << " / N of non-IoT group: " << n_non_iot_group <<std::endl;
	std::cout << std::endl << "Number of Stats: " << stats.size() << std::endl;
  for (uint32_t i = 1; i < stats.size(); i++) {
    if((stats[i].timeLastRxPacket.GetSeconds () - stats[i].timeFirstRxPacket.GetSeconds ()) > 0) {
      //std::cout << "Stats[" << i << "]'s statistics" << std::endl;
      //std::cout << "Throughput: " << stats[i].rxBytes * 8.0 / (stats[i].timeLastRxPacket.GetSeconds () - stats[i].timeFirstRxPacket.GetSeconds ()) / 1000 << " Kbps" << std::endl;
      //std::cout << "Mean delay: " << stats[i].delaySum.GetSeconds () / stats[i].rxPackets << "s" << std::endl << std::endl;
			single_throughput = stats[i].rxBytes * 8.0 / (stats[i].timeLastRxPacket.GetSeconds () - stats[i].timeFirstRxPacket.GetSeconds ()) / 1000;
			total_throughput += single_throughput;
    }

		total_jitter_sum += stats[i].jitterSum.GetSeconds();
		total_delay_sum += stats[i].delaySum.GetSeconds ();
		total_rx_packet += stats[i].rxPackets;
  }
	std::cout << "Total throughput: " << total_throughput << "Kbps" << std::endl;
	std::cout << "Mean delay of all stats: " << total_delay_sum / total_rx_packet << "s" << std::endl;
	std::cout << "Mean jitter of all stats: " << total_jitter_sum / total_rx_packet << "s" << std::endl;

	/*
	//Gnuplot ...continued 
	gnuplot.AddDataset (dataset);

	// Open the plot file.
	std::ofstream plotFile (plotFileName.c_str());

	// Write the plot file.
	gnuplot.GenerateOutput (plotFile);

	// Close the plot file.
	plotFile.close ();
	*/

	/*uint64_t totalRxBytesCounter = 0;
	for (uint32_t i = 0; i < sinkApps.GetN (); i++) {
		Ptr <Application> app = sinkApps.Get (i);
		Ptr <PacketSink> pktSink = DynamicCast <PacketSink> (app);
		totalRxBytesCounter += pktSink->GetTotalRx ();
	}*/

	/*double averageThroughput = ((global_sink->GetTotalRx ()) / (global_stop_time));

	NS_LOG_UNCOND ("\n--------------------------------\nQueueDisc Type: "
								 << queueDiscType
								 << "\nNumber of IoT/Non-IoT devices connected to the edge device: "
								 << 21*n_iot_group
								 << "\nAverage throughput by global sink: "
								 << averageThroughput
								 << " Bytes/sec");
	NS_LOG_UNCOND ("--------------------------------");*/
	
	if (training_mode == false) {
		monitor->SerializeToXmlFile("iot-topology-" + queueDiscType + "-testMode-datarate"+ p2p_data_rate + "-delay" + p2p_delay + "-iot" +std::to_string(n_iot_group)+ "-scale" + std::to_string((int)(rewards_weight * 10)) + "-" + std::to_string((int)round(global_stop_time)) + "s-flowMonitor.xml", true, true);
	}
	else {
		monitor->SerializeToXmlFile("iot-topology-" + queueDiscType + "-datarate"+ p2p_data_rate + "-delay" + p2p_delay + "-iot" +std::to_string(n_iot_group)+ "-scale" + std::to_string((int)(rewards_weight * 10)) + "-" + std::to_string((int)round(global_stop_time)) + "s-episode" + std::to_string(episode) + "-flowMonitor.xml", true, true);
	}
	QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();

	std::cout << std::endl << "<" << queueDiscType << ">" << " Queue Disc Stats from the AP Interface on edge device" << std::endl;
	
	std::cout << st << std::endl;

	std::cout << "Destroying the simulation..." << std::endl;
	Simulator::Destroy();
	return 0;

}




