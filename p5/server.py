#!/usr/bin/env python3
import socket
import numpy
import struct
import sys
from scipy.io import wavfile
import webrtcvad


if len(sys.argv) != 2:
    print("Missing port number")
else:
    udp_port = int(sys.argv[1])

    s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    s.bind( ('0.0.0.0', udp_port))
    print ("Waiting to receive data from client")

    sr = 16000
    data = b''
    frame_ms = 30
          
    while (True):
        raw, addr = s.recvfrom(1024)
        data += raw
        print ("Received " + str(len(data)) + "bytes data from client")

        size = 320
        c = list(data[i : i + size] for i in range(0, len(data), size))

        if (len(c[-1]) != 320):
            #c = c[:-1]
            del c[-1]
            
        vad = webrtcvad.Vad(3)

        for item in c:
            if (vad.is_speech(item, sr)):
                print("Have voice")
            else:
                print("No voice")

    s.close()


