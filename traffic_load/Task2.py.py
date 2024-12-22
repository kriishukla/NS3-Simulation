import os
import pandas as pd
import matplotlib.pyplot as plt

# Define paths relative to the script's location
script_dir = os.path.dirname(os.path.abspath(__file__))  # Get the script's directory
base_path = os.path.join(script_dir, "results")  # Path to the results folder

# Traffic loads and seeds configuration
traffic_loads = list(range(1, 31))
seeds = list(range(1, 11))

# Initialize dictionaries to store the aggregated results
results = {
    'traffic_load': [],
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
        
        for line in lines:
            parts = line.split()
            if len(parts) > 0:
                event_type = parts[0]
                time = float(parts[1])
                packet_id = parts[-1]
                
                if event_type == 't':
                    sent_packets.append((time, packet_id))
                elif event_type == 'r':
                    received_packets.append((time, packet_id))
        
        # Calculate metrics
        total_sent = len(sent_packets)
        total_received = len(received_packets)
        
        pdr = total_received / total_sent if total_sent > 0 else 0
        
        delays = [
            recv_time - next(sent_time for sent_time, sent_id in sent_packets if sent_id == recv_id)
            for recv_time, recv_id in received_packets
        ]
        
        e2e_delay = sum(delays) / len(delays) if delays else 0
        throughput = (total_received * 8 * 1024) / (sent_packets[-1][0] - sent_packets[0][0]) if sent_packets else 0
        
        return pdr, e2e_delay, throughput
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return None, None, None

# Collect and aggregate results for each traffic load
for traffic_load in traffic_loads:
    pdrs = []
    e2e_delays = []
    throughputs = []
    
    for seed in seeds:
        trace_file = os.path.join(base_path, f'output_{traffic_load}_{seed}.tr')
        
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

        results['traffic_load'].append(traffic_load)
        results['pdr'].append(avg_pdr)
        results['e2e_delay'].append(avg_e2e_delay)
        results['throughput'].append(avg_throughput)

        # Print the results for the current traffic load
        print(f"Traffic Load: {traffic_load}")
        print(f"Average PDR: {avg_pdr}")
        print(f"Average End-to-End Delay: {avg_e2e_delay} seconds")
        print(f"Average Throughput: {avg_throughput / 1000} kbps")
        print()

# Convert results to DataFrame
df_results = pd.DataFrame(results)

fig, axs = plt.subplots(3, 1, figsize=(10, 15))

# PDR vs Traffic Load
axs[0].plot(df_results['traffic_load'], df_results['pdr'], marker='o')
axs[0].set_title('PDR vs Traffic Load')
axs[0].set_xlabel('Traffic Load')
axs[0].set_ylabel('PDR')
axs[0].grid(True)
axs[0].legend(['PDR'])
axs[0].set_ylim([0, 100])
axs[0].set_xlim([0, 30])

# Save PDR plot
pdr_plot_path = os.path.join(script_dir, "pdr_vs_traffic_load.png")
plt.savefig(pdr_plot_path)

# E2E Delay vs Traffic Load
axs[1].plot(df_results['traffic_load'], df_results['e2e_delay'], marker='o')
axs[1].set_title('End-to-End Delay vs Traffic Load')
axs[1].set_xlabel('Traffic Load')
axs[1].set_ylabel('End-to-End Delay (s)')
axs[1].grid(True)
axs[1].legend(['E2E Delay'])
axs[1].set_ylim([5, 10])
axs[1].set_xlim([0, 30])

# Save E2E Delay plot
e2e_delay_plot_path = os.path.join(script_dir, "e2e_delay_vs_traffic_load.png")
plt.savefig(e2e_delay_plot_path)

# Throughput vs Traffic Load
axs[2].plot(df_results['traffic_load'], df_results['throughput'], marker='o')
axs[2].set_title('Throughput vs Traffic Load')
axs[2].set_xlabel('Traffic Load')
axs[2].set_ylabel('Throughput (kbps)')
axs[2].grid(True)
axs[2].legend(['Throughput'])
axs[2].set_ylim([0, max(df_results['throughput']) * 1.1])
axs[2].set_xlim([0, 30])

# Save Throughput plot
throughput_plot_path = os.path.join(script_dir, "throughput_vs_traffic_load.png")
plt.savefig(throughput_plot_path)

plt.tight_layout()
