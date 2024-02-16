#!/usr/bin/env python3

from time import sleep
import argparse
import json
import os
import psutil
import subprocess
import sys
import tempfile


class MyParser(argparse.ArgumentParser):
    def error(self, message):
        sys.stderr.write('error: %s\n' % message)
        self.print_help()
        sys.exit(2)


class colors:
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    END = '\033[0m'


def printV(string):
    if args.verbose:
        print(string)


def check_network_address(search_addr):
    addresses = psutil.net_if_addrs()
    stats = psutil.net_if_stats()
    available_networks = {}
    for intface, addr_list in addresses.items():
        if any(getattr(addr, 'address').startswith("169.254") for addr in addr_list):
            continue
        elif intface in stats and getattr(stats[intface], "isup"):
            for addr in addr_list:
                if addr.address == search_addr:
                    available_networks[intface] = addr.address
                    break

    return available_networks


def start_process(executable, config_file):
    with open(config_file, "r") as f:
        config = json.load(f)
    # Assuming your config files have parameters as key-value pairs
    # Adjust this according to your actual config file structure
    command = [executable]  # Start with the executable
    for key, value in config.items():
        command.append(f"--{key}")
        command.append(str(value))
    subprocess.Popen(command)


def parse_dca_config(config) -> dict[str, str]:
    with open(config, 'r') as file:
        content = file.read()
        parsed_config = json.loads(content)
        dca1000_config = parsed_config["DCA1000Config"]
        eth_config = dca1000_config["ethernetConfig"]
        eth_config_update = dca1000_config["ethernetConfigUpdate"]
        ip = eth_config["DCA1000IPAddress"]
        ip_update = eth_config_update["DCA1000IPAddress"]
        mac_update = eth_config_update["DCA1000MACAddress"]
        # print(f"ip: {ip} <-> new ip: {ip_update}, mac: {mac_update}")
        return {"json": parsed_config, "ip": ip, "ip_update": ip_update, "mac_update": mac_update}


def check_ip_route(destination):
    try:
        result = subprocess.run(['ip', 'route', 'show', destination], capture_output=True, text=True, timeout=5)
        if result.returncode == 0 and destination in result.stdout:
            return True
        else:
            return False
    except subprocess.TimeoutExpired:
        print("Timeout: Command took too long to execute.")
        return False
    except Exception as e:
        print("Error occurred:", e)
        return False


def run_dca_cmd(cmd, jsonfile, timeout=5):
    result = 0
    try:
        # timeout request to cut waiting time
        result = subprocess.run([executables['dca'], cmd, jsonfile], capture_output=True, timeout=timeout, check=True)
        result = result.returncode
    except subprocess.TimeoutExpired:
        result = 2
    except subprocess.CalledProcessError:
        print(f"{executables['dca']} returned error")
        result = 1
    return result


def main(config_files, executables):
    for name in executables:
        if not os.path.exists(os.path.join("bin", name)):
            print(f"Binary {name} not available. Please build the project.")
            exit(1)
    for config_file in config_files:
        start_process("bin/setup_dca_1000", config_file)


def setup(config_files, executables):
    print("Running setup routine\n")
    print(f"Found {len(config_files)} config files: {config_files}")
    print(f"Going to setup {len(config_files)} radar modules:")
    reprogrammed = False
    for i, config_file_path in enumerate(config_files):
        parsed_dict = parse_dca_config(config_file_path)
        print(f"{colors.CYAN}Radar {i+1}{colors.END}: Please connect a single radar which should be assigned the ip address: {parsed_dict['ip_update']}")
        if input(" Press enter when DCA ethernet is connected or 'c' to cancel...") == 'c':
            continue

        # with tempfile.TemporaryDirectory(prefix="setup_dca_1000_") as temp_dir:
        fixed_temp_dir = os.path.join(tempfile.gettempdir(), "setup_dca_1000")
        if not os.path.exists(fixed_temp_dir):
            os.makedirs(fixed_temp_dir)

        temp_file_path = os.path.join(fixed_temp_dir, f"dca_cf.json")
        # always start at base ip
        ip_tmp = "192.168.33.180"
        config_temp = parsed_dict['json']
        config_temp["DCA1000Config"]["ethernetConfig"]["DCA1000IPAddress"] = ip_tmp
        ip_found = None
        print("Checking DCA reachability, please be patient...")
        for i in range(11):
            # dump json to temp config and test
            with open(temp_file_path, 'w') as temp_file:
                json.dump(config_temp, temp_file, indent=2)
            printV(f"IP: {ip_tmp}")
            if len(check_network_address("192.168.33.30")) == 0:
                print("Waiting for network with fixed ip 192.168.33.30...")
            while len(check_network_address("192.168.33.30")) == 0:
                sleep(1)
            result = run_dca_cmd("reset_fpga", temp_file_path, timeout=2)
            if result == 0:
                print(f"found dca on ip: {ip_tmp}")
                ip_found = ip_tmp
                break
            else:
                pass
            ip_tmp = ip_tmp.split(".")
            ip_tmp[-1] = str(int(ip_tmp[-1]) + 1)
            ip_tmp = ".".join(ip_tmp)
            config_temp["DCA1000Config"]["ethernetConfig"]["DCA1000IPAddress"] = ip_tmp
        if ip_found is None:
            print("Could not reach DCA, check connection or repower")
            continue

        if ip_tmp != parsed_dict['ip_update'] or args.force:
            # create update json and program dca
            config_temp["DCA1000Config"]["ethernetConfigUpdate"]["DCA1000IPAddress"] = parsed_dict['ip_update']
            with open(temp_file_path, 'w') as temp_file:
                json.dump(config_temp, temp_file, indent=2)
            result = run_dca_cmd("eeprom", temp_file_path)
            if result == 0:
                run_dca_cmd("reset_fpga", temp_file_path, timeout=2)
                reprogrammed = True
                print(f"reprogrammed dca to ip: {parsed_dict['ip_update']}")
                config_temp["DCA1000Config"]["ethernetConfig"]["DCA1000IPAddress"] = parsed_dict['ip_update']
                with open(config_file_path, 'w') as orig_config_file:
                    json.dump(config_temp, orig_config_file, indent=2)
            else:
                print("Error reprogramming")
                printV(result.stdout)
                printV(result.stderr)
        else:
            print("IPs match, nothing todo, continuing...")

        if i < len(config_files):
            input(" Press enter when DCA ethernet disconnected...")
    if reprogrammed:
        print("\nRepower the radar module(s) to finish programming.")


if __name__ == "__main__":
    parser = MyParser(description='mmWave Radar Wrapper')
    parser.add_argument('setup', help='Run setup routine', nargs='?')
    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose mode')
    parser.add_argument('-f', '--force', action='store_true', help='Force')
    args = parser.parse_args()

    executables = {"dca": "bin/setup_dca_1000", "radar": "bin/setup_radar"}
    config_files = ["setup_dca_1000/cf_0.json", "setup_dca_1000/cf_1.json", "setup_dca_1000/cf_2.json"]

    if len(sys.argv) == 1:
        parser.print_help(sys.stderr)
        sys.exit(1)

    if args.setup:
        setup(config_files, executables)
    else:
        main(config_files, executables)
