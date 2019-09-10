Scenario Easley
===============


Attributes
----------


Root
~~~~

+-------------------+--------------------+---------------------------------+
| Name              | Type               | Description                     |
+===================+====================+=================================+
| ``logOnFile``     | ``bool``           | Enable file logging             |
+-------------------+--------------------+---------------------------------+
| ``phyMode``       | ``string``         | Transmission mode               |
+-------------------+--------------------+---------------------------------+
| ``duration``      | ``double``         | Simulation duration, in seconds |
+-------------------+--------------------+---------------------------------+
| ``drones``        | *Array of objects* | See `below <#drone>`_           |
+-------------------+--------------------+---------------------------------+
| ``ZSPs``          | *Array of objects* | See `below <#zsp>`_             |
+-------------------+--------------------+---------------------------------+
| ``logComponents`` | *Array of strings* | See :doc:`log-components`       |
+-------------------+--------------------+---------------------------------+


Drone
~~~~~

+--------------------------+--------------------+----------------------------+
| Name                     | Type               | Description                |
+==========================+====================+============================+
| ``speedCoefficients``    | *Array of double*  | Speed coefficients         |
+--------------------------+--------------------+----------------------------+
| ``applicationStartTime`` | ``double``         | Application start time     |
+--------------------------+--------------------+----------------------------+
| ``applicationStopTime``  | ``double``         | Application stop time      |
+--------------------------+--------------------+----------------------------+
| ``trajectory``           | *Array of objects* | See `below <#trajectory>`_ |
+--------------------------+--------------------+----------------------------+


Trajectory
~~~~~~~~~~

+--------------+-------------------+-----------------------------------------+
| Name         | Type              | Description                             |
+==============+===================+=========================================+
| ``position`` | *Array of double* | A list of Beziér knots                  |
+--------------+-------------------+-----------------------------------------+
| ``interest`` | ``double``        | *for future purposes*                   |
+--------------+-------------------+-----------------------------------------+
| ``restTime`` | ``double``        | Time to rest at each knot, in seconds   |
+--------------+-------------------+-----------------------------------------+


ZSP
~~~

+--------------+------------+--------------+
| Name         | Type       | Description  |
+==============+============+==============+
| ``position`` | ``double`` | ZSP position |
+--------------+------------+--------------+


Example
-------

This scenario needs a JSON configuration to be run, here is an example
according to the aformentioned specification::

    {
      "logOnFile": true,
      "phyMode": "DsssRate1Mbps",
      "duration": 10.0,
      "drones": [
        {
          "speedCoefficients": [1.0, 0.0],
          "applicationStartTime": 1.0,
          "applicationStopTime": 9.0,
          "trajectory": [
            {
              "position": [0.0, 0.0, 0.0],
              "interest": 0,
              "restTime": 3.0
            },
            {
              "position": [10.0, 10.0, 0.0],
              "interest": 1
            },
            {
              "position": [20.0, 0.0, 0.0],
              "interest": 0,
              "restTime": 3.0
            }
          ]
        }
      ],
      "ZSPs": [
        {
          "position": [10.0, 10.0, 0.0]
        }
      ],
      "logComponents": [
        "Curve",
        "ConstantAccelerationFlight",
        "Planner",
        "ConstantAccelerationDroneMobilityModel",
        "DroneServer",
        "DroneClient",
        "ScenarioConfigurationHelper"
      ]
    }

You can run this scenario by invoking ``waf``, for example::

    $  ./waf --run "drone-scenario-easley --config=./easley.json"
