Scenario Babbage
================


Attributes
----------

+---------------+--------------+---------+---------------------------------+
| Name          | Type         | Default | Description                     |
+---------------+--------------+---------+---------------------------------+
| ``numDrones`` | ``uint32_t`` | 2       | Number of drones                |
+---------------+--------------+---------+---------------------------------+
| ``duration``  | ``double``   | 120.0   | Simulation duration, in seconds |
+---------------+--------------+---------+---------------------------------+

Example
-------

You can run this scenario by invoking ``waf``, for example::

    $  ./waf --run "drone-scenario-babbage --numDrones=3 --duration=600"
