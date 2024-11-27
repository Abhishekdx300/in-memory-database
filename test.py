# import subprocess
# import time
# import threading
# import random

# # Configuration
# COMMAND = "./client"  # Path to your client binary
# NUM_THREADS = 10      # Number of parallel threads
# NUM_OPERATIONS = 1000 # Operations per thread
# KEY_RANGE = 100       # Range for random keys
# SCORE_RANGE = 100     # Range for random scores

# # Function to execute a single command
# def execute_command(command):
#     start_time = time.time()
#     process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#     stdout, stderr = process.communicate()
#     end_time = time.time()
#     latency = end_time - start_time
#     return latency, stdout.decode('utf-8').strip(), stderr.decode('utf-8').strip()

# # Worker function to test commands
# def worker(results, operations):
#     for _ in range(operations):
#         # Randomly choose a command to execute
#         cmd_type = random.choice([
#             "get", "set", "del", "pexpire", "pttl", 
#             "zadd", "zrem", "zscore", "zquery"
#         ])
#         key = f"key{random.randint(1, KEY_RANGE)}"
#         name = f"name{random.randint(1, KEY_RANGE)}"
#         score = str(random.randint(1, SCORE_RANGE))
#         milsec = str(random.randint(1000, 5000))
#         offset = random.randint(0, 5)  # Example offset
#         limit = 4  # Limit must be even

#         # Construct the command based on type
#         if cmd_type == "get":
#             command = [COMMAND, "get", key]
#         elif cmd_type == "set":
#             command = [COMMAND, "set", key]
#         elif cmd_type == "del":
#             command = [COMMAND, "del", key]
#         elif cmd_type == "pexpire":
#             command = [COMMAND, "pexpire", key, milsec]
#         elif cmd_type == "pttl":
#             command = [COMMAND, "pttl", key]
#         elif cmd_type == "zadd":
#             command = [COMMAND, "zadd", key, score, name]
#         elif cmd_type == "zrem":
#             command = [COMMAND, "zrem", key, name]
#         elif cmd_type == "zscore":
#             command = [COMMAND, "zscore", key, name]
#         elif cmd_type == "zquery":
#             command = [COMMAND, "zquery", key, score, name, str(offset), str(limit)]

#         # Execute the command and record the latency
#         latency, stdout, stderr = execute_command(command)
#         results.append((cmd_type, latency, stdout, stderr))

# # Measure throughput and latency
# def measure_performance():
#     threads = []
#     results = []

#     # Start worker threads
#     for _ in range(NUM_THREADS):
#         t = threading.Thread(target=worker, args=(results, NUM_OPERATIONS))
#         threads.append(t)
#         t.start()

#     # Wait for all threads to complete
#     for t in threads:
#         t.join()

#     # Analyze results
#     total_operations = len(results)
#     total_time = sum(latency for _, latency, _, _ in results)
#     avg_latency = total_time / total_operations
#     throughput = total_operations / total_time

#     # Aggregate results by command type
#     command_stats = {}
#     for cmd_type, latency, stdout, stderr in results:
#         if cmd_type not in command_stats:
#             command_stats[cmd_type] = {"count": 0, "total_latency": 0.0}
#         command_stats[cmd_type]["count"] += 1
#         command_stats[cmd_type]["total_latency"] += latency

#     # Print results
#     print(f"Total Operations: {total_operations}")
#     print(f"Average Latency: {avg_latency:.6f} seconds")
#     print(f"Throughput: {throughput:.2f} operations/second\n")

#     for cmd_type, stats in command_stats.items():
#         avg_cmd_latency = stats["total_latency"] / stats["count"]
#         print(f"Command: {cmd_type}")
#         print(f"  Operations: {stats['count']}")
#         print(f"  Average Latency: {avg_cmd_latency:.6f} seconds")
#         print()

# if __name__ == "__main__":
#     measure_performance()


import subprocess
import time
import threading
import random
import csv
import os

# Configuration
COMMAND = "./client"  # Path to your client binary
NUM_THREADS = 10      # Number of parallel threads
NUM_OPERATIONS = 1000 # Operations per thread
KEY_RANGE = 100       # Range for random keys
SCORE_RANGE = 100     # Range for random scores
RESULT_FILE = "performance_results.csv"  # Output CSV file

# Function to execute a single command
def execute_command(command):
    start_time = time.time()
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    end_time = time.time()
    latency = end_time - start_time
    return latency, stdout.decode('utf-8').strip(), stderr.decode('utf-8').strip()

# Worker function to test commands
def worker(results, operations):
    for _ in range(operations):
        # Randomly choose a command to execute
        cmd_type = random.choice([
            "get", "set", "del", "pexpire", "pttl", 
            "zadd", "zrem", "zscore", "zquery"
        ])
        key = f"key{random.randint(1, KEY_RANGE)}"
        name = f"name{random.randint(1, KEY_RANGE)}"
        score = str(random.randint(1, SCORE_RANGE))
        milsec = str(random.randint(1000, 5000))
        offset = random.randint(0, 5)  # Example offset
        limit = 4  # Limit must be even

        # Construct the command based on type
        if cmd_type == "get":
            command = [COMMAND, "get", key]
        elif cmd_type == "set":
            command = [COMMAND, "set", key]
        elif cmd_type == "del":
            command = [COMMAND, "del", key]
        elif cmd_type == "pexpire":
            command = [COMMAND, "pexpire", key, milsec]
        elif cmd_type == "pttl":
            command = [COMMAND, "pttl", key]
        elif cmd_type == "zadd":
            command = [COMMAND, "zadd", key, score, name]
        elif cmd_type == "zrem":
            command = [COMMAND, "zrem", key, name]
        elif cmd_type == "zscore":
            command = [COMMAND, "zscore", key, name]
        elif cmd_type == "zquery":
            command = [COMMAND, "zquery", key, score, name, str(offset), str(limit)]

        # Execute the command and record the latency
        latency, stdout, stderr = execute_command(command)
        results.append((cmd_type, latency, stdout, stderr))

# Measure throughput and latency
def measure_performance():
    threads = []
    results = []

    # Start worker threads
    for _ in range(NUM_THREADS):
        t = threading.Thread(target=worker, args=(results, NUM_OPERATIONS))
        threads.append(t)
        t.start()

    # Wait for all threads to complete
    for t in threads:
        t.join()

    # Analyze results
    total_operations = len(results)
    total_time = sum(latency for _, latency, _, _ in results)
    avg_latency = total_time / total_operations
    throughput = total_operations / total_time

    # Aggregate results by command type
    command_stats = {}
    for cmd_type, latency, stdout, stderr in results:
        if cmd_type not in command_stats:
            command_stats[cmd_type] = {"count": 0, "total_latency": 0.0}
        command_stats[cmd_type]["count"] += 1
        command_stats[cmd_type]["total_latency"] += latency

    # Prepare data for CSV
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    rows = [
        [timestamp, NUM_THREADS, NUM_OPERATIONS, total_operations, avg_latency, throughput, cmd_type, stats["count"], stats["total_latency"] / stats["count"]]
        for cmd_type, stats in command_stats.items()
    ]

    # Write to CSV file
    file_exists = os.path.isfile(RESULT_FILE)
    with open(RESULT_FILE, mode='a', newline='') as file:
        writer = csv.writer(file)
        if not file_exists:
            # Write header if the file is being created
            writer.writerow(["Timestamp","Total Threads","Operations per Thread" ,"Total Operations", "Average Latency", "Throughput", "Command", "Command Count", "Command Average Latency"])
        writer.writerows(rows)

    # Print results to console
    print(f"Total Operations: {total_operations}")
    print(f"Average Latency: {avg_latency:.6f} seconds")
    print(f"Throughput: {throughput:.2f} operations/second\n")

    for cmd_type, stats in command_stats.items():
        avg_cmd_latency = stats["total_latency"] / stats["count"]
        print(f"Command: {cmd_type}")
        print(f"  Operations: {stats['count']}")
        print(f"  Average Latency: {avg_cmd_latency:.6f} seconds")
        print()

if __name__ == "__main__":
    measure_performance()
