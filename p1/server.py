import socket
import sys
import numpy
import struct
import librosa

port = int(sys.argv[1])
s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.bind( ('0.0.0.0', port))
raw, addr = s.recvfrom(100000)
s.close()


a2 = struct.unpack('<'+'f'*int(len(raw)/4), raw)
a = numpy.array(a2, dtype=numpy.float32)



librosa.output.write_wav('elephant2.wav', a, len(a))
