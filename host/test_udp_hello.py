import socket


# ADDR = "224.1.1.1"
ADDR = "10.0.64.64"
PORT = 7000

MULTICAST_TTL = 2

# open UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
# Allow sending to same host
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Set multicast TTL
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)
# Bind to all interfaces
sock.bind(('', PORT))

print("Sending 'Hello, world!' to", ADDR, "on port", PORT)
sock.sendto(b"Hello, world!", (ADDR, PORT))

sock.close()

print("Done")