# This script can be used as a notebook to programatically design a scenario.
#
# This program is free software you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program if not , write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# %%
import json
import plotly.graph_objects as go
from py_design_support import create_roi, gen_trajectory, Point

# %%
colors = ['mediumblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue',
          'red', 'darkorange', 'darkorange', 'darkorange', 'darkorange', 'darkorange',
          'green', 'lime', 'lime', 'lime', 'lime', 'lime', 'yellow']

rois = [
    create_roi(10, 256, 0, 88, 66, 28, 'lightgrey'),
    create_roi(268, 254, 0, 38, 66, 29, 'lightgrey'),
    create_roi(168, 254, 0, 82, 66, 24, 'lightgrey'),
    create_roi(114, 254, 0, 40, 68, 24, 'lightgrey'),
    create_roi(114, 172, 0, 38, 66, 29, 'lightgrey'),
    create_roi(8, 172, 0, 88, 68, 30, 'lightgrey'),
    create_roi(266, 170, 0, 38, 66, 25, 'lightgrey'),
    create_roi(168, 170, 0, 84, 66, 28, 'lightgrey'),
    create_roi(266, 90, 0, 38, 62, 30, 'lightgrey'),
    create_roi(168, 90, 0, 82, 64, 27, 'lightgrey'),
    create_roi(112, 90, 0, 38, 64, 24, 'lightgrey'),
    create_roi(6, 90, 0, 88, 64, 27, 'lightgrey'),
    create_roi(6, 12, 0, 144, 60, 24, 'lightgrey'),
    create_roi(262, 10, 0, 40, 62, 29, 'lightgrey'),
    create_roi(162, 10, 0, 88, 60, 28, 'lightgrey'),
]

h = 50  # drone height, in meters

drones = [
    [
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h),
        Point(x=160, y=80, z=h),
        Point(x=150, y=10, z=h),
        Point(x=0, y=10, z=h),
        Point(x=0, y=80, z=h),
        Point(x=70, y=80, z=h)
    ],
    [
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h),
        Point(x=100, y=250, z=h),
        Point(x=256, y=250, z=h),
        Point(x=256, y=160, z=h),
        Point(x=0, y=160, z=h),
        Point(x=0, y=250, z=h)
    ],
    [
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h),
        Point(x=256, y=0, z=h),
        Point(x=256, y=10, z=h),
        Point(x=256, y=250, z=h),
        Point(x=310, y=250, z=h),
        Point(x=310, y=0, z=h),
        Point(x=310, y=0, z=h)
    ]
]

print(f'Number of drones: {len(drones)}')

# Ground Users
GUs = [
    [Point(x=float(x), y=float(y), z=1.0)]
    for x in range(10, 310, 30)
    for y in [80.0, 160.0, 247.0]
]

GUs += [
    [Point(x=float(x), y=float(y), z=1.0)]
    for y in range(10, 310, 30)
    for x in [157.0, 256.0, 310.0]
]

print(f'Number of Ground Users: {len(GUs)}')

trs = gen_trajectory(drones + GUs, palette=colors)
fig = go.Figure(data=[*rois, *trs])
fig.update_layout(scene_aspectmode='data')
fig.show()

# %% Export drone trajectories in JSON
data = json.dumps([{
    "netDevices": [
        {
            "type": "lte",
            "networkLayer": 3,
            "role": "UE",
            "bearers": [
                {
                    "type": "GBR_CONV_VIDEO",
                    "bitrate": {
                        "guaranteed": {
                            "downlink": 50e6,
                            "uplink": 20e6
                        },
                        "maximum": {
                            "downlink": 50e6,
                            "uplink": 20e6
                        }
                    }
                }
            ],
            "phy": {
                "TxPower": 24.0,
                "EnableUplinkPowerControl": False
            }
        },
        {
            "type": "wifi",
            "networkLayer": 0,
            "macLayer": {
                "name": "ns3::ApWifiMac",
                "attributes": [{
                    "name": "Ssid",
                    "value": "wifi-1"
                }]
            }
        }
    ],
    "mobilityModel": {
        "name": "ns3::ParametricSpeedDroneMobilityModel",
        "attributes": [{
                "name": "SpeedCoefficients",
                "value": [5.0]
        },
            {
                "name": "FlightPlan",
                "value": [
                    {
                        "position": [float(p.x), float(p.y), float(p.z)],
                        "interest": int(p.interest),
                        "restTime": 1.0
                    }
                    for p in d
                ]
        }
        ]
    },
    "applications": [{
        "name": "ns3::NatApplication",
        "attributes": [{
            "name": "InternalNetDeviceId",
            "value": 2
        },
            {
            "name": "ExternalNetDeviceId",
            "value": 0
        }]
    }],
    "mechanics": {
        "name": "ns3::Drone",
        "attributes": [
            {
                "name": "Mass",
                "value": 0.750
            },
            {
                "name": "RotorDiskArea",
                "value": 0.18
            },
            {
                "name": "DragCoefficient",
                "value": 0.08
            }
        ]
    },
    "battery": {
        "name": "ns3::LiIonEnergySource",
        "attributes": [
            {
                "name": "LiIonEnergySourceInitialEnergyJ",
                "value": 5000.0
            },
            {
                "name": "LiIonEnergyLowBatteryThreshold",
                "value": 0.2
            }
        ]
    },
    "peripherals": []
}
    for d in drones[0:3]])

with open('paper_s3-drones.json', 'w') as f:
    f.write(data)


# %% Divide GUs in 3 wifi networks and export them in JSON
gus_wifi1 = [d for d in drones[3:] if d[0].x <= 190.0 and d[0].y <= 160.0]
gus_wifi2 = [d for d in drones[3:] if d[0].x <= 190.0 and d[0].y > 160.0]
gus_wifi3 = [d for d in drones[3:] if d[0].x > 190.0 and d[0].y > 160.0]

data = json.dumps([{
    "netDevices": [
        {
            "type": "wifi",
            "networkLayer": 0,
            "macLayer": {
                "name": "ns3::StaWifiMac",
                "attributes": [{
                    "name": "Ssid",
                    "value": "wifi-1"
                }]
            }
        }
    ],
    "mobilityModel": {
        "name": "ns3::ConstantPositionMobilityModel",
        "attributes": [{
            "name": "Position",
            "value": [float(d[0].x), float(d[0].y), float(d[0].z)]
        }]
    },
    "applications": [{
        "name": "ns3::TcpStorageClientApplication",
        "attributes": [
            {
                "name": "StartTime",
                "value": 1.0
            },
            {
                "name": "StopTime",
                "value": 101.0
            },
            {
                "name": "Address",
                "value": "200.0.0.1"
            },
            {
                "name": "PayloadSize",
                "value": 1024
            }
        ]
    }],
    "mechanics": {
        "name": "ns3::Drone",
        "attributes": [
            {
                "name": "Mass",
                "value": 0.750
            },
            {
                "name": "RotorDiskArea",
                "value": 0.18
            },
            {
                "name": "DragCoefficient",
                "value": 0.08
            }
        ]
    },
    "battery": {
        "name": "ns3::LiIonEnergySource",
        "attributes": [
            {
                "name": "LiIonEnergySourceInitialEnergyJ",
                "value": 5000.0
            },
            {
                "name": "LiIonEnergyLowBatteryThreshold",
                "value": 0.2
            }
        ]
    },
    "peripherals": [
        {
            "name": "ns3::StoragePeripheral",
            "attributes": [
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "Capacity",
                    "value": 3300000000
                },
                {
                    "name": "InitialRemainingCapacity",
                    "value": 3300000000
                }
            ]
        },
        {
            "name": "ns3::InputPeripheral",
            "attributes": [
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "DataRate",
                    "value": 2000000.0
                },
                {
                    "name": "HasStorage",
                    "value": "true"
                }
            ]
        }
    ]
}
    for d in gus_wifi1
])

with open('paper_s3-gus_wifi1.json', 'w') as f:
    f.write(data)

data = json.dumps([{
    "netDevices": [
        {
            "type": "wifi",
            "networkLayer": 1,
            "macLayer": {
                "name": "ns3::StaWifiMac",
                "attributes": [{
                    "name": "Ssid",
                    "value": "wifi-2"
                }]
            }
        }
    ],
    "mobilityModel": {
        "name": "ns3::ConstantPositionMobilityModel",
        "attributes": [{
            "name": "Position",
            "value": [float(d[0].x), float(d[0].y), float(d[0].z)]
        }]
    },
    "applications": [{
        "name": "ns3::TcpStorageClientApplication",
        "attributes": [
            {
                "name": "StartTime",
                "value": 1.0
            },
            {
                "name": "StopTime",
                "value": 101.0
            },
            {
                "name": "Address",
                "value": "200.0.0.1"
            },
            {
                "name": "PayloadSize",
                "value": 1024
            }
        ]
    }],
    "mechanics": {
        "name": "ns3::Drone",
        "attributes": [
            {
                "name": "Mass",
                "value": 0.750
            },
            {
                "name": "RotorDiskArea",
                "value": 0.18
            },
            {
                "name": "DragCoefficient",
                "value": 0.08
            }
        ]
    },
    "battery": {
        "name": "ns3::LiIonEnergySource",
        "attributes": [
            {
                "name": "LiIonEnergySourceInitialEnergyJ",
                "value": 5000.0
            },
            {
                "name": "LiIonEnergyLowBatteryThreshold",
                "value": 0.2
            }
        ]
    },
    "peripherals": [
        {
            "name": "ns3::StoragePeripheral",
            "attributes": [
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "Capacity",
                    "value": 3300000000
                },
                {
                    "name": "InitialRemainingCapacity",
                    "value": 3300000000
                }
            ]
        },
        {
            "name": "ns3::InputPeripheral",
            "attributes": [
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "DataRate",
                    "value": 2000000.0
                },
                {
                    "name": "HasStorage",
                    "value": "true"
                }
            ]
        }
    ]
}
    for d in gus_wifi2
])

with open('paper_s3-gus_wifi2.json', 'w') as f:
    f.write(data)

data = json.dumps([{
    "netDevices": [
        {
            "type": "wifi",
            "networkLayer": 2,
            "macLayer": {
                "name": "ns3::StaWifiMac",
                "attributes": [{
                    "name": "Ssid",
                    "value": "wifi-3"
                }]
            }
        }
    ],
    "mobilityModel": {
        "name": "ns3::ConstantPositionMobilityModel",
        "attributes": [{
            "name": "Position",
            "value": [float(d[0].x), float(d[0].y), float(d[0].z)]
        }]
    },
    "applications": [{
        "name": "ns3::TcpStorageClientApplication",
        "attributes": [
            {
                "name": "StartTime",
                "value": 1.0
            },
            {
                "name": "StopTime",
                "value": 101.0
            },
            {
                "name": "Address",
                "value": "200.0.0.1"
            },
            {
                "name": "PayloadSize",
                "value": 1024
            }
        ]
    }],
    "mechanics": {
        "name": "ns3::Drone",
        "attributes": [
            {
                "name": "Mass",
                "value": 0.750
            },
            {
                "name": "RotorDiskArea",
                "value": 0.18
            },
            {
                "name": "DragCoefficient",
                "value": 0.08
            }
        ]
    },
    "battery": {
        "name": "ns3::LiIonEnergySource",
        "attributes": [
            {
                "name": "LiIonEnergySourceInitialEnergyJ",
                "value": 5000.0
            },
            {
                "name": "LiIonEnergyLowBatteryThreshold",
                "value": 0.2
            }
        ]
    },
    "peripherals": [
        {
            "name": "ns3::StoragePeripheral",
            "attributes": [
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "Capacity",
                    "value": 3300000000
                },
                {
                    "name": "InitialRemainingCapacity",
                    "value": 3300000000
                }
            ]
        },
        {
            "name": "ns3::InputPeripheral",
            "attributes":[
                {
                    "name": "PowerConsumption",
                    "value": [0, 1.0, 5.0]
                },
                {
                    "name": "DataRate",
                    "value": 2000000.0
                },
                {
                    "name": "HasStorage",
                    "value": "true"
                }
            ]
        }
    ]
  }
  for d in gus_wifi3
])

with open('paper_s3-gus_wifi3.json', 'w') as f:
    f.write(data)
