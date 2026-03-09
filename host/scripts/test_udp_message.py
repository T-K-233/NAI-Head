import socket

import numpy as np

# Device IP (unicast). Must match firmware, e.g. 172.28.0.64
ADDR = "172.28.0.64"
PORT = 7000


class UDPMessageIndex:
    FACE_ANGLE_X = 1
    FACE_ANGLE_Y = 2
    FACE_ANGLE_Z = 3
    BROW_HEIGHT_LEFT = 4
    BROW_HEIGHT_RIGHT = 5
    EYE_OPEN_LEFT = 6
    EYE_OPEN_RIGHT = 7
    EYE_LEFT_X = 8
    EYE_LEFT_Y = 9
    EYE_RIGHT_X = 10
    EYE_RIGHT_Y = 11


NUM_MESSAGE_FIELDS = 12

message = np.zeros(NUM_MESSAGE_FIELDS, dtype=np.float32)

message[UDPMessageIndex.EYE_OPEN_LEFT] = 1.0
message[UDPMessageIndex.EYE_OPEN_RIGHT] = 1.0

message[UDPMessageIndex.BROW_HEIGHT_LEFT] = 0.0
message[UDPMessageIndex.BROW_HEIGHT_RIGHT] = 0.0

message[UDPMessageIndex.EYE_LEFT_X] = 0.5
message[UDPMessageIndex.EYE_LEFT_Y] = 0.5
message[UDPMessageIndex.EYE_RIGHT_X] = 0.5
message[UDPMessageIndex.EYE_RIGHT_Y] = 0.5

# Direct UDP socket (unicast)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("Sending states to", ADDR, "on port", PORT)
sock.sendto(message.tobytes(), (ADDR, PORT))

sock.close()

print("Done")