import time
import socket

import numpy as np


# ADDR = "224.1.1.1"
ADDR = "172.28.0.64"
PORT = 7000

MULTICAST_TTL = 2

N_STATES = 12


states = np.zeros(N_STATES, dtype=np.float32)

states[7] = 1.0
states[8] = 1.0
states[9] = 0.0

# gesture
states[10] = 3

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(("", PORT))


control_eyes = True

sock.sendto(states.tobytes(), (ADDR, PORT))
time.sleep(0.03)

sock.close()

print("Done")