import socket


# Device IP (unicast). Must match firmware, e.g. 172.28.0.64
ADDR = "172.28.0.64"
PORT = 7000

# Direct UDP socket (unicast)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("Sending 'Hello, world!' to", ADDR, "on port", PORT)
sock.sendto(b"Hello, world!", (ADDR, PORT))

sock.close()

print("Done")
