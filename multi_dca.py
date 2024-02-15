#!/usr/bin/env python3

import argparse
import json
import os
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
        return {"ip": ip, "ip_update": ip_update, "mac_update": mac_update}


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
    for i, config in enumerate(config_files):
        parsed_dict = parse_dca_config(config)
        print(f"{colors.CYAN}Radar {i+1}{colors.END}: Please connect a single radar which should be assigned the ip adress: {parsed_dict['ip_update']}")
        input(" Press enter when connected...")

        with tempfile.TemporaryDirectory() as temp_dir:
            # create temp config file to change ip and test for dca reachability
            with tempfile.NamedTemporaryFile(dir=temp_dir, prefix="dca_config_", suffix=".json") as temp_config:                
                ip_tmp = parsed_dict['ip']
                config_temp = config
                ip_found = None
                print("Checking DCA reachability, please be patient...")
                for i in range(11):
                    # dump json to temp config and test
                    json.dump(config_temp, temp_config)
                    try:
                        result = subprocess.run([executables['dca'], "reset_fpga", temp_config], capture_output=True, timeout=0.5)
                        if result.returncode == 0:
                            print(f"found dca on ip: {ip_tmp}")
                            ip_found = ip_tmp
                            break
                    except subprocess.TimeoutExpired:
                        continue
                    ip_tmp = ip_tmp.split(".")
                    ip_tmp[-1] = str(int(ip_tmp[-1]) + 1)
                    ip_tmp = ".".join(ip_tmp)
                    config_temp["DCA1000Config"]["ethernetConfig"]["DCA1000IPAddress"] = ip_tmp

                if ip_found is None:
                    print("Could not reach DCA, check connection")

        # print(f"{colors.CYAN}Radar {i}{colors.END}: Please disconnect the radar")
        # input(" Press enter when disconnected...")


if __name__ == "__main__":
    parser = MyParser(description='mmWave Radar Wrapper')
    parser.add_argument('setup', help='Run setup routine', nargs='?')
    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose mode')
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
