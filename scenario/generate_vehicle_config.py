#!/usr/bin/env python3
"""
Script to generate JSON configuration for vehicle tracing
Creates a simple scenario with multiple vehicles following different mobility patterns
"""

import json
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))


def generate_vehicles():
    """Generate vehicle configuration with different mobility models"""

    cars = []

    # Vehicle 1: Constant Velocity Model
    car1 = {
        "type": "Car",
        "netDevices": [],
        "mobilityModel": {
            "name": "ns3::ConstantVelocityMobilityModel",
            "attributes": [
                {
                    "name": "Position",
                    "value": "0.0:0.0:1.5",  # x:y:z starting position (1.5m height for car)
                },
                {
                    "name": "Velocity",
                    "value": "20.0:0.0:0.0",  # 20 m/s along x-axis (72 km/h)
                },
            ],
        },
    }

    # Vehicle 2: Random Walk 2D
    car2 = {
        "type": "Car",
        "netDevices": [],
        "mobilityModel": {
            "name": "ns3::RandomWalk2dMobilityModel",
            "attributes": [
                {"name": "Position", "value": "100.0:100.0:1.5"},
                {
                    "name": "Speed",
                    "value": "ns3::UniformRandomVariable[Min=5.0|Max=15.0]",  # 5-15 m/s
                },
                {
                    "name": "Direction",
                    "value": "ns3::UniformRandomVariable[Min=0.0|Max=6.283185307]",  # 0-2π radians
                },
                {
                    "name": "Bounds",
                    "value": "0|200|0|200",  # Rectangle bounds
                },
            ],
        },
    }

    # Vehicle 3: Waypoint Mobility
    car3 = {
        "type": "Car",
        "netDevices": [],
        "mobilityModel": {
            "name": "ns3::WaypointMobilityModel",
            "attributes": [
                {"name": "Position", "value": "50.0:50.0:1.5"},
                {"name": "LazyNotify", "value": True},
            ],
        },
    }

    # Vehicle 4: Constant Position (parked)
    car4 = {
        "type": "Car",
        "netDevices": [],
        "mobilityModel": {
            "name": "ns3::ConstantPositionMobilityModel",
            "attributes": [{"name": "Position", "value": "150.0:75.0:1.5"}],
        },
    }

    # Vehicle 5: Another Constant Velocity with different direction
    car5 = {
        "type": "Car",
        "netDevices": [],
        "mobilityModel": {
            "name": "ns3::ConstantVelocityMobilityModel",
            "attributes": [
                {"name": "Position", "value": "200.0:200.0:1.5"},
                {
                    "name": "Velocity",
                    "value": "-10.0:-10.0:0.0",  # Moving southwest at ~50 km/h
                },
            ],
        },
    }

    cars = [car1, car2, car3, car4, car5]

    return cars


def create_full_scenario():
    """Create the complete scenario JSON configuration"""

    cars = generate_vehicles()

    scenario = {
        "name": "vehicle-tracing-example",
        "resultsPath": "../results/",
        "logOnFile": True,
        "duration": 300,  # 5 minutes simulation
        "staticNs3Config": [],
        "world": {"size": {"X": "500", "Y": "500", "Z": "50"}, "buildings": []},
        "phyLayer": [{"type": "none"}],
        "macLayer": [],
        "cars": cars,
    }

    return scenario


def main():
    """Main function to generate and save the configuration"""

    print("Generating vehicle tracing configuration...")

    scenario = create_full_scenario()

    # Calculate total vehicles
    total_cars = len(scenario["cars"])

    print(f"\nGenerated {total_cars} vehicles:")
    for i, car in enumerate(scenario["cars"]):
        mobility_name = car["mobilityModel"]["name"].replace("ns3::", "")
        print(f"- Vehicle {i+1}: {mobility_name}")

    # Save to file
    output_file = "vehicle-tracing-example.json"
    with open(output_file, "w") as f:
        json.dump(scenario, f, indent=2)

    print(f"\nConfiguration saved to: {output_file}")
    print(f"To run: ./ns3 run scenario -- --config=../scenario/{output_file}")


if __name__ == "__main__":
    main()
