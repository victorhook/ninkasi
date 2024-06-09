import socket
import struct
import sys

IP = 'ninkasi'
PORT = 2346


if __name__ == '__main__':
    sock = socket.socket()
    sock.settimeout(None)
    sock.connect((IP, PORT))
    
    while True:
        try:
            data = sock.recv(6 * 4)
            telem = struct.unpack_from('<ffffff', data)
            sys.stdout.write('\r' + str(telem) + '\n')
            sys.stdout.flush()
        except Exception:
            pass