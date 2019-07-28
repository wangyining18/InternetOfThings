#!/usr/bin/env python3

################################################
#
#  This file will generate a CPP Header file 
#  consisting of the data from a WAV file
#
################################################


import argparse as ap
import librosa
import numpy as np

parser = ap.ArgumentParser( description='C++ Wave Generator')
parser.add_argument('-wav', '--wav', type=str, default='wave.wav')
parser.add_argument('-sr', '--sample_rate', type=int, default=16000) 
parser.add_argument( '-cpp', '--cpp_header', type=str, default='wave.h',
                        help='Output CPP Header file') 

args = parser.parse_args()

# load the WAV file
print ('Loading WAV data: ' + args.wav)
ys, ysr = librosa.core.load( args.wav, mono=True, sr=args.sample_rate)

# generate our CPP file
print ('Generating CPP Header file: ' + args.cpp_header)
header_name = '__' + args.cpp_header.upper().replace('.', '_') + '__'
with open(args.cpp_header, 'w') as f:
    f.write('#ifndef ' + header_name + ' \n')
    f.write('#define ' + header_name + ' \n')
    
    f.write('const int WAVE_SAMPLE_RATE = ' + str(ysr) + ';\n')
    f.write('const int WAVE_LENGTH = ' + str(len(ys)) + ';\n')
    f.write('const float WAVE_DATA [] = {\n\t')
    f.write(',\n\t'.join([ str(y) for y in ys] ))
    f.write('\n\t};\n')

    f.write('#endif\n')


