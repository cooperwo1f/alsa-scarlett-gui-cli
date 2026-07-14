#!/usr/bin/env python3
"""
MIDI-to-Scarlett Mixer Control Script

Monitors MIDI input and dynamically adjusts mixer gains based on CC values.
Requires: python3-mido (MIDI I/O library)

Install: pip install mido

Usage:
    python3 midi_mixer.py [--port PORT] [--device DEVICE]

Example:
    # List available MIDI ports:
    python3 midi_mixer.py --list-ports
    
    # Listen on "LPK25 MIDI In" and control "Mix A 1 Input 1 Playback Volume":
    python3 midi_mixer.py --port "LPK25 MIDI In" --mapping config.json
"""

import subprocess
import sys
import argparse
import json
from pathlib import Path

try:
    import mido
except ImportError:
    print("Error: mido not installed. Install with: pip install mido")
    sys.exit(1)


class MidiMixerBridge:
    """Bridge between MIDI controller and Scarlett mixer via CLI"""
    
    def __init__(self, cli_tool="alsa-scarlett-gui"):
        self.cli_tool = cli_tool
        self.mapping = {}  # CC number -> control name
        self.ranges = {}   # Control name -> (min_val, max_val)
    
    def load_mapping(self, config_file):
        """Load CC-to-control mapping from JSON config file"""
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)
            self.mapping = config.get('mapping', {})
            self.ranges = config.get('ranges', {})
            print(f"Loaded config from {config_file}")
            print(f"Mapped {len(self.mapping)} controls")
        except FileNotFoundError:
            print(f"Config file not found: {config_file}")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"Invalid JSON in {config_file}: {e}")
            sys.exit(1)
    
    def list_available_ports(self):
        """Print available MIDI input ports"""
        print("Available MIDI input ports:")
        for i, port in enumerate(mido.get_input_names()):
            print(f"  {i}: {port}")
    
    def list_mixer_controls(self):
        """List available mixer controls from the device"""
        try:
            result = subprocess.run(
                [self.cli_tool, "--cli", "list"],
                capture_output=True,
                text=True,
                check=True
            )
            print(result.stdout)
        except subprocess.CalledProcessError as e:
            print(f"Error listing controls: {e.stderr}")
            sys.exit(1)
    
    def set_control(self, control_name, value):
        """Set a mixer control via CLI"""
        try:
            result = subprocess.run(
                [self.cli_tool, "--cli", "set", control_name, str(int(value))],
                capture_output=True,
                text=True,
                check=False
            )
            if result.returncode != 0:
                print(f"Error setting {control_name}: {result.stderr.strip()}")
            else:
                print(f"✓ {control_name} = {int(value)}")
        except Exception as e:
            print(f"Error executing CLI: {e}")
    
    def cc_to_db(self, cc_value, control_name):
        """Convert MIDI CC value (0-127) to dB for the control"""
        if control_name not in self.ranges:
            # Default: map 0-127 to -80 to 0 dB
            return -80 + (cc_value / 127.0) * 80
        
        min_db, max_db = self.ranges[control_name]
        return min_db + (cc_value / 127.0) * (max_db - min_db)
    
    def run(self, port_name):
        """Main loop: listen for MIDI and control mixer"""
        print(f"Connecting to MIDI port: {port_name}")
        
        try:
            with mido.open_input(port_name) as inport:
                print(f"Connected! Listening for CC messages...")
                print("(Press Ctrl+C to exit)\n")
                
                for msg in inport:
                    if msg.type == 'control_change':
                        cc = str(msg.control)
                        if cc in self.mapping:
                            control_name = self.mapping[cc]
                            db_value = self.cc_to_db(msg.value, control_name)
                            self.set_control(control_name, db_value)
                    
                    elif msg.type == 'program_change':
                        # Could implement preset switching here
                        print(f"Program change: {msg.program}")
        
        except KeyboardInterrupt:
            print("\nExiting...")
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)


def create_example_config():
    """Generate an example configuration file"""
    config = {
        "description": "MIDI CC to Scarlett mixer mapping",
        "mapping": {
            "1": "Mix A 1 Input 1 Playback Volume",
            "2": "Mix A 1 Input 2 Playback Volume",
            "3": "Mix A 1 Input 3 Playback Volume",
            "4": "Mix A 1 Input 4 Playback Volume"
        },
        "ranges": {
            "Mix A 1 Input 1 Playback Volume": [-80, 0],
            "Mix A 1 Input 2 Playback Volume": [-80, 0],
            "Mix A 1 Input 3 Playback Volume": [-80, 0],
            "Mix A 1 Input 4 Playback Volume": [-80, 0]
        }
    }
    
    with open("midi_mixer_config.json", "w") as f:
        json.dump(config, f, indent=2)
    
    print("Example config created: midi_mixer_config.json")


def main():
    parser = argparse.ArgumentParser(
        description="Control Scarlett mixer via MIDI CC messages"
    )
    parser.add_argument(
        "--port",
        help="MIDI input port name"
    )
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="List available MIDI ports and exit"
    )
    parser.add_argument(
        "--list-controls",
        action="store_true",
        help="List available mixer controls and exit"
    )
    parser.add_argument(
        "--mapping",
        default="midi_mixer_config.json",
        help="Path to CC mapping config file (default: midi_mixer_config.json)"
    )
    parser.add_argument(
        "--create-config",
        action="store_true",
        help="Create an example config file and exit"
    )
    parser.add_argument(
        "--cli-tool",
        default="alsa-scarlett-gui",
        help="Path to CLI tool (default: alsa-scarlett-gui)"
    )
    
    args = parser.parse_args()
    
    if args.create_config:
        create_example_config()
        return 0
    
    bridge = MidiMixerBridge(args.cli_tool)
    
    if args.list_ports:
        bridge.list_available_ports()
        return 0
    
    if args.list_controls:
        bridge.list_mixer_controls()
        return 0
    
    if not args.port:
        print("Error: --port required (use --list-ports to see available ports)")
        return 1
    
    if Path(args.mapping).exists():
        bridge.load_mapping(args.mapping)
    else:
        print(f"Warning: Config file not found: {args.mapping}")
        print("Creating example config...")
        create_example_config()
        print("Edit midi_mixer_config.json and try again.")
        return 1
    
    bridge.run(args.port)
    return 0


if __name__ == "__main__":
    sys.exit(main())
