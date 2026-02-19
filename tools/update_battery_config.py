import re
import os
import glob
import sys

# Constants
SCENARIO_DIR = "scenario"
OLD_CLASS_NAME = "ns3::energy::LiIonEnergySource"
NEW_CLASS_NAME = "ns3::energy::GenericBatteryModel"
DEFAULT_VOLTAGE = 4.18

def get_full_voltage(content):
    # Try to find InitialCellVoltage to use for conversion
    # Pattern: { "name": "InitialCellVoltage", "value": 4.1 }
    # Flexible on quotes around value and whitespace
    match = re.search(r'\{\s*"name"\s*:\s*"InitialCellVoltage"\s*,\s*"value"\s*:\s*"?([0-9eE\.-]+)"?\s*\}', content)
    if match:
        return float(match.group(1))
    return DEFAULT_VOLTAGE

def calculate_capacity(energy_joules, voltage):
    return energy_joules / (voltage * 3600.0)

def process_file(filepath):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}", file=sys.stderr)
        return

    # Check if the file contains the target class
    if OLD_CLASS_NAME not in content:
        return

    print(f"Updating {filepath}...")
    original_content = content

    # 1. Replace the class name
    content = content.replace(OLD_CLASS_NAME, NEW_CLASS_NAME)

    # 2. Find and determine Full Voltage for calculation (before we modify attributes)
    v_full = get_full_voltage(content)

    # 3. Attributes Replacement Logic

    # helper for replacements
    def replace_initial_energy(match):
        val_str = match.group(1)
        try:
            energy = float(val_str)
            capacity = calculate_capacity(energy, v_full)
            # Create replacement string
            # We add BatteryType here as well
            # match.group(0) is the entire curly brace block { ... }
            # We replace it with two blocks: MaxCapacity and BatteryType

            # We try to keep basic indentation from the match if possible, or just default 20 spaces?
            # The regex matched the whole block. We can just return the new string.

            new_block = (f'{{\n'
                         f'                        "name": "MaxCapacity",\n'
                         f'                        "value": {capacity}\n'
                         f'                    }},\n'
                         f'                    {{\n'
                         f'                        "name": "BatteryType",\n'
                         f'                        "value": "LION_LIPO"\n'
                         f'                    }}')
            return new_block
        except ValueError:
            return match.group(0) # return original if parse fails

    # Regex for LiIonEnergySourceInitialEnergyJ
    # Matches: { "name": "LiIonEnergySourceInitialEnergyJ", "value": 5000 }
    # Handles multiline and flexible spacing
    energy_pattern = re.compile(r'\{\s*"name"\s*:\s*"LiIonEnergySourceInitialEnergyJ"\s*,\s*"value"\s*:\s*"?([0-9eE\.-]+)"?\s*\}', re.DOTALL)

    # Check if we have InitialEnergy
    if energy_pattern.search(content):
        content = energy_pattern.sub(replace_initial_energy, content)
    else:
        # If no InitialEnergy, we still need to add BatteryType.
        # But for now, we assume InitialEnergy is present as it's a mandatory attribute usually.
        pass

    # 4. Rename other attributes
    # LiIonEnergyLowBatteryThreshold -> LowBatteryThreshold
    content = content.replace('"LiIonEnergyLowBatteryThreshold"', '"LowBatteryThreshold"')

    # InitialCellVoltage -> FullVoltage
    content = content.replace('"InitialCellVoltage"', '"FullVoltage"')

    # NominalCellVoltage -> NominalVoltage
    content = content.replace('"NominalCellVoltage"', '"NominalVoltage"')

    # ExpCellVoltage -> ExponentialVoltage
    content = content.replace('"ExpCellVoltage"', '"ExponentialVoltage"')

    # RatedCapacity -> MaxCapacity
    content = content.replace('"RatedCapacity"', '"MaxCapacity"')

    # ExpCapacity -> ExponentialCapacity
    content = content.replace('"ExpCapacity"', '"ExponentialCapacity"')

    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)

def main():
    # Find all JSON files in the scenario directory
    search_path = os.path.join(SCENARIO_DIR, "*.json")
    files = glob.glob(search_path)

    if not files:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(script_dir)
        search_path = os.path.join(project_root, SCENARIO_DIR, "*.json")
        files = glob.glob(search_path)

    print(f"Found {len(files)} JSON files in {search_path}")

    for f in files:
        process_file(f)

if __name__ == "__main__":
    main()
