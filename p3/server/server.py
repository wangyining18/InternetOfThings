
import socket
import numpy
import struct
import librosa
import sys

from scipy.io import wavfile

port = int(sys.argv[1])
 
# Receiving datat from client
s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.bind( ('0.0.0.0', port))
print("Waiting to receive data from client")

wav_file = []
norm_array = numpy.ones(16000)
for i in range(10):
    raw, addr = s.recvfrom(4000)
    data = struct.unpack('<'+'i'*int(len(raw)/4), raw)
    wav_file.append(data)

print("Received data from client")


# Calculate the file hz
a = numpy.array(wav_file, numpy.float32)
avg = (numpy.sum(a)) / 16000
norm_array = (a - avg)/avg
librosa.output.write_wav('raw.wav', norm_array, 16000)


m = numpy.ones((1, 10))/10
padded = numpy.pad(m[0,:], (0, len(norm_array)-10), 'constant')
convolved_data = numpy.convolve(norm_array,mask,'same')
output = wavfile.write("filter.wav", 16000, convolved_data)
ms = (numpy.mean(convolved_data ** 2))
print("MS value is " + str(ms))


threshold = 0.01

# Return ms to the client
if (ms < threshold):
    print("440Hz has been detected")
    s.sendto(bytes([1]), addr)    
else:
    print ("440Hz has not been detected")
    s.sendto(bytes([0]), addr) 
    
s.close()

