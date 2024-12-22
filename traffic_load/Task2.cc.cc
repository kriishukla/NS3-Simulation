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
#include <filesystem>

using namespace ns3;
namespace fs = std::filesystem;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

// Function to parse traffic pairs
std::vector<std::pair<uint32_t, uint32_t>> ParseTrafficPairs(const std::string &pairsStr) {
    std::vector<std::pair<uint32_t, uint32_t>> pairs;
    std::stringstream ss(pairsStr);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        size_t pos = pair.find(':');
        if (pos != std::string::npos) {
            uint32_t src = std::stoi(pair.substr(0, pos));
            uint32_t dest = std::stoi(pair.substr(pos + 1));
            pairs.emplace_back(src, dest);
        }
    }
    return pairs;
}

int main(int argc, char *argv[]) {
    LogComponentEnable("SecondScriptExample", LOG_LEVEL_ALL);
    NS_LOG_UNCOND("Starting simulation...");

    // Simulation parameters
    uint32_t cols = 10;
    uint32_t rows = 10;
    uint32_t numNodes = 80;
    uint32_t nodeSpacing = 3;
    uint32_t duration = 20;
    float seed = 1.0;
    uint32_t run = 5;
    uint32_t packetRate = 10;
    uint32_t packetSize = 1024;
    uint32_t trafficLoad = 1;
    std::string trafficPairs;
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
    cmd.AddValue("trafficPairs", "Traffic pairs in the format source1:dest1,source2:dest2,...", trafficPairs);
    cmd.AddValue("showSimTime", "Show simulation time", showSimTime);
    cmd.AddValue("trafficLoad", "Different traffic flows", trafficLoad);
    cmd.Parse(argc, argv);

    std::vector<std::pair<uint32_t, uint32_t>> trafficPairsVector = ParseTrafficPairs(trafficPairs);

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

    ApplicationContainer serverApps;
    ApplicationContainer clientApps;

    for (const auto &pair : trafficPairsVector) {
        uint32_t sourceNode = pair.first;
        uint32_t destinationNode = pair.second;

        if (sourceNode >= numNodes || destinationNode >= numNodes) {
            NS_LOG_ERROR("Invalid source or destination node index: " << sourceNode << " -> " << destinationNode);
            continue;
        }

        NS_LOG_UNCOND("Setting up traffic from node " << sourceNode << " to node " << destinationNode);

        UdpServerHelper server(4000);
        serverApps.Add(server.Install(c.Get(destinationNode)));

        UdpClientHelper client(interfaces.GetAddress(destinationNode), 4000);
        client.SetAttribute("MaxPackets", UintegerValue(100000000));
        client.SetAttribute("Interval", TimeValue(Seconds(1.0 / packetRate)));
        client.SetAttribute("PacketSize", UintegerValue(packetSize));
        clientApps.Add(client.Install(c.Get(sourceNode)));
    }

    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(duration - 1));
    clientApps.Start(Seconds(0.0));
    clientApps.Stop(Seconds(duration));

    std::string resultsDir = "./traffic_load/results";
    if (!fs::exists(resultsDir)) {
        fs::create_directories(resultsDir);
    }

    AsciiTraceHelper ascii;
    std::ostringstream oss;
    oss << resultsDir << "/output_" << trafficLoad << "_" << seed << ".tr";
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream(oss.str()));

    NS_LOG_UNCOND("Running simulation...");
    Simulator::Stop(Seconds(duration));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_UNCOND("Simulation complete.");

    return 0;
}
