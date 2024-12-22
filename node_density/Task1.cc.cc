#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/olsr-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem> // C++17 for checking/creating directories

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int main(int argc, char *argv[]) {
    uint32_t cols = 10;
    uint32_t rows = 10;
    uint32_t numNodes = 20;
    uint32_t nodeSpacing = 3;
    uint32_t duration = 20;
    float seed = 1.0;
    uint32_t run = 5;
    uint32_t packetRate = 10;
    uint32_t packetSize = 1024;
    uint32_t sourceNode = 1;
    uint32_t destinationNode = 18;
    bool showSimTime = true;

    CommandLine cmd;
    cmd.AddValue("cols", "Columns of nodes", cols);
    cmd.AddValue("rows", "Rows of nodes", rows);
    cmd.AddValue("numNodes", "Number of nodes", numNodes);
    cmd.AddValue("nodeSpacing", "Spacing between neighboring nodes", nodeSpacing);
    cmd.AddValue("duration", "Duration of simulation", duration);
    cmd.AddValue("seed", "Random seed for simulation", seed);
    cmd.AddValue("run", "Simulation run", run);
    cmd.AddValue("packetRate", "Packets transmitted per second", packetRate);
    cmd.AddValue("packetSize", "Packet size", packetSize);
    cmd.AddValue("sourceNode", "Number of source node", sourceNode);
    cmd.AddValue("destinationNode", "Number of destination node", destinationNode);
    cmd.AddValue("showSimTime", "Show simulation time", showSimTime);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));

    SeedManager::SetSeed(seed);
    SeedManager::SetRun(run);

    NodeContainer c;
    c.Create(numNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ArfWifiManager");

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

    OlsrHelper olsr;
    InternetStackHelper internet;
    internet.SetRoutingHelper(olsr);
    internet.Install(c);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpServerHelper server(4000);
    ApplicationContainer serverApps = server.Install(c.Get(destinationNode));
    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(duration - 1));

    UdpClientHelper client(interfaces.GetAddress(destinationNode), 4000);
    client.SetAttribute("MaxPackets", UintegerValue(100000000));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0 / packetRate)));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApps = client.Install(c.Get(sourceNode));
    clientApps.Start(Seconds(0.0));
    clientApps.Stop(Seconds(duration - 1));

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(1.0),
                                  "MinY", DoubleValue(1.0),
                                  "DeltaX", DoubleValue(nodeSpacing),
                                  "DeltaY", DoubleValue(nodeSpacing),
                                  "GridWidth", UintegerValue(cols));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0, (cols * nodeSpacing) + 1, 0, (rows * nodeSpacing) + 1)),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=10.0]"),
                              "Distance", DoubleValue(30));
    mobility.Install(c);

    // Define relative paths and ensure directory exists
    std::string base_path = "./node_density/results/";
    std::filesystem::create_directories(base_path); // Ensure 'results' directory exists

    // Create dynamic filename for trace file
    std::ostringstream oss;
    oss << base_path << "output_" << numNodes << "_" << seed << ".tr";
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream(oss.str()));

    Simulator::Stop(Seconds(duration));
    Simulator::Run();
    Simulator::Destroy();

    std::cout << "Trace file saved to: " << oss.str() << std::endl;

    return 0;
}
