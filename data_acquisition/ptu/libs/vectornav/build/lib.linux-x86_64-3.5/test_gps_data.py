
# import sys
from vnpy import *


def main():
    s = VnSensor()
    s.connect('/dev/ttyUSB0', 115200)
    print(s.read_model_number())
    ecef_register = s.read_gps_solution_ecef()
    print(ecef_register.position.x)
    print(ecef_register.position.y)
    print(ecef_register.position.z)
    return

main()