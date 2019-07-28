import librosa
import numpy
import struct
import socket
import sys

port = int(sys.argv[1])

# UDP Server
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.bind(('0.0.0.0', port))
raw, addr = s.recvfrom(102400)
s.close()

# Converting from Bytes
ysBytes = struct.unpack('<'+'f'*int(len(raw)/4), raw)
ys = numpy.array(ysBytes, dtype=numpy.float32)

# Saving WAV Files
librosa.output.write_wav('elephant.wav', ys, len(ys))
