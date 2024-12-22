import os
import pandas as pd
import matplotlib.pyplot as plt

# Define paths
base_path = './task_one/results'
node_densities = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
seeds = list(range(1, 11))

# Ensure results directory exists
if not os.path.exists(base_path):
    print(f"Error: Results directory '{base_path}' does not exist.")
    exit(1)

# Initialize dictionaries to store the aggregated results
results = {
    'node_density': [],
    'pdr': [],
    'e2e_delay': [],
    'throughput': []
}

# Function to parse the trace file and extract metrics
def parse_trace_file(file_path):
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        sent_packets = []
        received_packets = []
        first_sent_time = None
        last_received_time = None
        total_received_bytes = 0
        
        for line in lines:
            parts = line.split()
            if len(parts) > 0:
                event_type = parts[0]
                time = float(parts[1])
                packet_id = parts[-1]
                
                # Extract packet size from the trace line
                packet_size = 0
                for part in parts:
                    if part.startswith("length:"):
                        try:
                            packet_size = int(part.split(':')[1])
                        except ValueError:
                            packet_size = 0
                        break
                
                if event_type == 't':
                    sent_packets.append((time, packet_id))
                    if first_sent_time is None:
                        first_sent_time = time
                elif event_type == 'r':
                    received_packets.append((time, packet_size, packet_id))
                    total_received_bytes += packet_size
                    last_received_time = time
        
        # Calculate metrics
        total_sent = len(sent_packets)
        total_received = len(received_packets)
        
        pdr = total_received / total_sent if total_sent > 0 else 0
        
        delays = [recv_time - next(sent_time for sent_time, sent_id in sent_packets if sent_id == recv_id)
                  for recv_time, recv_size, recv_id in received_packets if any(sent_id == recv_id for sent_time, sent_id in sent_packets)]
        
        e2e_delay = sum(delays) / len(delays) if delays else 0
        duration = last_received_time - first_sent_time if first_sent_time and last_received_time else 1
        throughput = (total_received_bytes * 8) / (1024 * duration) if duration > 0 else 0  # Throughput in Kbps
        
        return pdr, e2e_delay, throughput
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return None, None, None

# Collect and aggregate results for each node density
for numNodesVal in node_densities:
    pdrs = []
    e2e_delays = []
    throughputs = []
    
    for seedVal in seeds:
        trace_file = os.path.join(base_path, f'output_{numNodesVal}_{seedVal}.tr')
        
        if os.path.exists(trace_file):
            pdr, e2e_delay, throughput = parse_trace_file(trace_file)
            
            if pdr is not None:
                pdrs.append(pdr)
                e2e_delays.append(e2e_delay)
                throughputs.append(throughput)
    
    if pdrs and e2e_delays and throughputs:
        avg_pdr = sum(pdrs) / len(pdrs)
        avg_e2e_delay = sum(e2e_delays) / len(e2e_delays)
        avg_throughput = sum(throughputs) / len(throughputs)

        results['node_density'].append(numNodesVal)
        results['pdr'].append(avg_pdr)
        results['e2e_delay'].append(avg_e2e_delay)
        results['throughput'].append(avg_throughput)

        # Print the results for the current node density
        print(f"Node Density: {numNodesVal}")
        print(f"Average PDR: {avg_pdr}")
        print(f"Average End-to-End Delay: {avg_e2e_delay} seconds")
        print(f"Average Throughput: {avg_throughput} Kbps")
        print()

df_results = pd.DataFrame(results)

fig, axs = plt.subplots(3, 1, figsize=(10, 15))

# PDR vs Node Density
axs[0].plot(df_results['node_density'], df_results['pdr'], marker='o')
axs[0].set_title('PDR vs Node Density')
axs[0].set_xlabel('Node Density')
axs[0].set_ylabel('PDR')
axs[0].grid(True)
axs[0].legend(['PDR'])
axs[0].set_ylim([0, 1]) 
plt.savefig(os.path.join(base_path, 'pdr_vs_node_density.png'))

# E2E Delay vs Node Density
axs[1].plot(df_results['node_density'], df_results['e2e_delay'], marker='o')
axs[1].set_title('End-to-End Delay vs Node Density')
axs[1].set_xlabel('Node Density')
axs[1].set_ylabel('End-to-End Delay (s)')
axs[1].grid(True)
axs[1].legend(['E2E Delay'])
plt.savefig(os.path.join(base_path, 'e2e_delay_vs_node_density.png'))

# Throughput vs Node Density
axs[2].plot(df_results['node_density'], df_results['throughput'], marker='o')
axs[2].set_title('Throughput vs Node Density')
axs[2].set_xlabel('Node Density')
axs[2].set_ylabel('Throughput (Kbps)')
axs[2].grid(True)
axs[2].legend(['Throughput'])
plt.savefig(os.path.join(base_path, 'throughput_vs_node_density.png'))

plt.tight_layout()
