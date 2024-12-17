import time
import socket

import numpy as np
from cc.xboxcontroller import XboxController

# ADDR = "224.1.1.1"
ADDR = "172.28.0.64"
PORT = 7000

MULTICAST_TTL = 2

N_STATES = 10

stick = XboxController(0, deadzone=0, dampen=1e-2)



states = np.zeros(N_STATES, dtype=np.float32)

states[5] = -1

states[7] = 1.0
states[8] = 1.0

# open UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
# Allow sending to same host
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Set multicast TTL
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)
# Bind to all interfaces
sock.bind(('', PORT))


control_eyes = True

try:
    while True:
        stick.update()

        if stick.get_a_button():
            control_eyes = True
        if stick.get_b_button():
            control_eyes = False

        if control_eyes:
            states[3] = stick.get_left_x()
            states[4] = stick.get_left_y()
            states[5] = stick.get_right_x()
            states[6] = stick.get_right_y()

        else:
            states[0] = stick.get_left_x()
            states[1] = stick.get_left_y()
            states[2] = stick.get_right_x()

        states[7] = 1 - stick.get_left_trigger()
        states[8] = 1 - stick.get_right_trigger()

        if stick.get_dpad() == 0:
            states[9] = 2
        elif stick.get_dpad() == 180:
            states[9] = 1
        elif stick.get_dpad() == 90:
            states[9] = 0

        print(states)


        # print("Sending states to", ADDR, "on port", PORT)
        sock.sendto(states.tobytes(), (ADDR, PORT))

        time.sleep(0.03)
except KeyboardInterrupt:
    pass


sock.close()

print("Done")