#!/usr/bin/env python3
"""
Script to generate JSON configuration for LEO satellites
Reproduces the configuration from:
  satellites = orbit.Install ({ LeoOrbit (1200, 20, 32, 16),
                LeoOrbit (1180, 30, 12, 10) });
"""

import json
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))


def generate_leo_satellites():
    """Generate LEO satellite configuration based on orbital parameters"""

    # Define orbital configurations
    # LeoOrbit (altitude_km, inclination_deg, num_planes, sats_per_plane)
    orbits = [
        {"altitude": 1200, "inclination": 20, "planes": 32, "sats_per_plane": 16},
        {"altitude": 1180, "inclination": 30, "planes": 12, "sats_per_plane": 10},
    ]

    leo_sats = []
    sat_id = 0

    for orbit in orbits:
        altitude = orbit["altitude"]
        inclination = orbit["inclination"]
        num_planes = orbit["planes"]
        sats_per_plane = orbit["sats_per_plane"]

        # Calculate longitude spacing between planes
        longitude_spacing = 360.0 / num_planes

        # Calculate offset spacing within each plane
        offset_spacing = 360.0 / sats_per_plane

        for plane in range(num_planes):
            # Calculate longitude for this plane, normalized to [-180, 180]
            longitude = (plane * longitude_spacing) % 360
            if longitude > 180:
                longitude -= 360

            for sat in range(sats_per_plane):
                # Calculate offset for this satellite within the plane [0, 360]
                offset = sat * offset_spacing

                # Create satellite configuration
                satellite = {
                    "type": "LeoSat",
                    "netDevices": [],
                    "mobilityModel": {
                        "name": "ns3::GeoLeoOrbitMobility",
                        "attributes": [
                            {"name": "EarthSpheroidType", "value": "SPHERE"},
                            {"name": "Altitude", "value": float(altitude)},
                            {"name": "Inclination", "value": float(inclination)},
                            {"name": "Longitude", "value": longitude},
                            {"name": "Offset", "value": offset},
                            {"name": "RetrogradeOrbit", "value": False},
                        ],
                    },
                }

                leo_sats.append(satellite)
                sat_id += 1

                print(
                    f"Generated satellite {sat_id}: Orbit {len(leo_sats)//((num_planes*sats_per_plane))+1}, "
                    f"Plane {plane+1}/{num_planes}, Sat {sat+1}/{sats_per_plane}, "
                    f"Alt={altitude}km, Inc={inclination}°, Long={longitude:.1f}°, Off={offset:.1f}°"
                )

    return leo_sats


def create_full_scenario():
    """Create the complete scenario JSON configuration"""

    leo_sats = generate_leo_satellites()

    scenario = {
        "name": "leo-circular-orbit-tracing",
        "resultsPath": "../results/",
        "logOnFile": True,
        "duration": 500,
        "staticNs3Config": [
            {"name": "ns3::GeoLeoOrbitMobility::Precision", "value": "1s"}
        ],
        "world": {
            "size": {"X": "40000000", "Y": "40000000", "Z": "40000000"},
            "buildings": [],
        },
        "phyLayer": [{"type": "none"}],
        "macLayer": [],
        "leo-sats": leo_sats,
    }

    return scenario


def main():
    """Main function to generate and save the configuration"""

    print("Generating LEO satellite constellation configuration...")
    print("Configuration: LeoOrbit (1200, 20, 32, 16) + LeoOrbit (1180, 30, 12, 10)")
    print()

    scenario = create_full_scenario()

    # Calculate total satellites
    total_sats = len(scenario["leo-sats"])

    print(f"\nGenerated {total_sats} satellites:")

    # Save to file
    output_file = "../leo-circular-orbit-tracing-example.json"
    with open(output_file, "w") as f:
        json.dump(scenario, f, indent=2)

    print(f"\nConfiguration saved to: {output_file}")
    print(f"To run: ./ns3 run scenario -- --config=../{output_file}")


if __name__ == "__main__":
    main()
