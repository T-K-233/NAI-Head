import socket
import struct
import time

from VTubeStudioBridge import VTubeStudioAPI, InputParameter

client = VTubeStudioAPI()

client.authenticate()

N_ACTIONS = 10

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind to all interfaces on port 7000
server_address = ("", 8000)
sock.bind(server_address)

print(f"Starting UDP server on port {server_address[1]}")

counter = 10

while True:
    try:
        data, address = sock.recvfrom(4 * N_ACTIONS)

        states = struct.unpack("10f", data)
        # print(f"Received {len(data)} bytes from {address}")
        # print(f"Data: {states}")

        counter -= 1

        if counter == 0:
            print(f"Setting parameters: {states}")

            client.set_parameters(
                {
                    InputParameter.FaceAngleZ: -states[0] * 10,
                    InputParameter.FaceAngleY: -states[1] * 10,
                    InputParameter.FaceAngleX: -states[2] * 20,
                    InputParameter.EyeLeftX: -states[3] * 100,
                    InputParameter.EyeLeftY: -states[4] * 100,
                    InputParameter.EyeRightX: -states[5] * 100,
                    InputParameter.EyeRightY: -states[6] * 100,
                    InputParameter.EyeOpenLeft: states[8],
                    InputParameter.EyeOpenRight: states[7],
                }
            )
            counter = 10


        
        
    except KeyboardInterrupt:
        print("\nShutting down server...")
        break

sock.close()
client.close()


