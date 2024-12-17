import socket
import struct


N_STATES = 10

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind to all interfaces on port 7000
server_address = ("", 8000)
sock.bind(server_address)

print(f"Starting UDP server on port {server_address[1]}")

while True:
    try:
        # Receive data (buffer size of 1024 bytes)
        data, address = sock.recvfrom(4 * N_STATES)

        states = struct.unpack("10f", data)
        print(f"Received {len(data)} bytes from {address}")
        print(f"Data: {states}")
        
    except KeyboardInterrupt:
        print("\nShutting down server...")
        break

sock.close()


