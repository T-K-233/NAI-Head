import socket
import struct

ADDR = "224.1.1.1"
PORT = 7000

MULTICAST_TTL = 2

# open UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)

# Allow multiple sockets to use the same PORT number
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind to all interfaces
sock.bind(('', PORT))

# Join the multicast group
mreq = struct.pack('4sl', socket.inet_aton(ADDR), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print("Listening on", ADDR, "on port", PORT)
data, addr = sock.recvfrom(1024)
print("Received", data, "from", addr)

sock.close()

print("Done")