import socket


ADDR = "10.0.64.64"
PORT = 7000

# open UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# listen on all interfaces
sock.bind(("0.0.0.0", PORT))

while True:
    data, addr = sock.recvfrom(1024)
    print(f"Received: {data} from {addr}")
    sock.sendto(data, addr)
