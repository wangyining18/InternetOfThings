import librosa
import numpy
import struct
import socket
import sys

# read file
ys, sr = librosa.core.load(sys.argv[2], mono = True, sr = 16000)


# convert to bytes
b = struct.pack('<' + 'f' * len(ys), * ys)

# start udp client
port = int(sys.argv[1])
server_address = ('iot.lukefahr.org', port)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(b, server_address)
s.close()









