from math import *

def Geo2Mer(z_x, z_y, lat_deg, lon_deg):
    DEG2RAD = 0.01745329251994329547

    lat_rad = DEG2RAD * lat_deg
    lon_rad = DEG2RAD * lon_deg

    x = 1 / 2 / pi * 2 ** z_x * (lon_rad + pi)
    y = 1 / 2 / pi * 2 ** z_y * (pi - log(tan(pi / 4 + lat_rad / 2)))

    return x, y


A_lat = 40.51579
A_lon = 17.40314

#METER = pi / 174960
METER = 1.028888889e-5

B_lat = A_lat + METER
B_lon = A_lon + METER
print(B_lat, B_lon)

z_x = 25.059
z_y = 24.665

A_x, A_y = Geo2Mer(z_x, z_y, A_lat, A_lon)
B_x, B_y = Geo2Mer(z_x, z_y, B_lat, B_lon)
print(abs(B_x - A_x), abs(B_y - A_y))
