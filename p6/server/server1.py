#!/usr/bin/env python3
import socket
import numpy
import struct
import sys
import scipy.io.wavfile
import tensorflow as tf
import os


def write_wav_file(byte_buff, sr = 16000, fname="sound.wav"):
    output = wave.open(fname, 'wb')
    output.setparams((1, 2, sr, 0, 'NONE', 'not compressed'))
    output.writeframes(byte_buff)
    output.close()
    print("Done writting wav file -", fname)


udp_port = int(sys.argv[1])
    

#create a socket
s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.bind( ('0.0.0.0', udp_port))
print("Waiting to receive data from client")


while(True):
    
    #arguments = sys.argv[1]
    #port = int(arguments)
    #s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    #s.bind( ('0.0.0.0', port))
    acc = b''
    buff_sz = 1600
    while len(acc) < 32000:
        raw, addr = s.recvfrom(1600) #maybe 64000
        acc += raw
        print("received:", len(raw), " accumulated:", len(acc))
    print("received: ", len(acc))
    s.close()
    write_wav_file(acc)


    print ("VOice received")
    
    def load_graph(filename):
        with tf.gfile.GFile(filename, 'rb') as f:
            graph_def = tf.GraphDef()
            graph_def.ParseFromString(f.read())
            tf.import_graph_def(graph_def, name='')
    
    
    def load_labels(filename):
        return [line.rstrip() for line in tf.gfile.GFile(filename)]
    
    
    def run_graph(wav_data, labels, input_layer_name, output_layer_name, num_top_predictions):
        with tf.Session() as sess:
            softmax_tensor = sess.graph.get_tensor_by_name(output_layer_name)
            predictions, = sess.run(softmax_tensor, {input_layer_name: wav_data})
            
            top_k = predictions.argsort()[-num_top_predictions:][::-1]
            scores = []
            for node_id in top_k:
                human_string = labels[node_id]
                score = predictions[node_id]
                scores.append((human_string, score))
    
            return scores
    
    
    def label_wav( wav, labels, graph, input_name="wav_data:0", output_name="labels_softmax:0", how_many_labels=3):
        if not wav or not tf.gfile.Exists(wav):
            tf.logging.fatal('Audio file does not exist %s', wav)

        if not labels or not tf.gfile.Exists(labels):
            tf.logging.fatal('Labels file does not exist %s', labels)
    
        if not graph or not tf.gfile.Exists(graph):
            tf.logging.fatal('Graph file does not exist %s', graph)
    
        labels_list = load_labels(labels)
        
        load_graph(graph)
            
        with open(wav, 'rb') as wav_file:
            wav_data = wav_file.read() 
                
        return run_graph( wav_data, labels_list, input_name, output_name, how_many_labels)

            
    def prediction_label(x):
        if x == "on" or x== "off" or x == "silence":
            return x.upper()
        else:
            return "ERROR"    
    
    
    CWD = os.getcwd() + '/'
    WAV_FILE = CWD + "udp_recvd.wav"
    OTHER_WAVE = CWD + "learn_fbb56351_nohash_1.wav"
    GRAPH_OBJ_FILE = CWD + "./cnn_speech_train.pb"
    LABELS_TXT = CWD + "conv_labels.txt"

    
    
    scores = label_wav(WAV_FILE, LABELS_TXT, GRAPH_OBJ_FILE)
    print ("ALL SCORES: " + str(scores))
    
    
    scores = sorted(scores, key = lambda x: x[1])
    pred, confidence = scores[-1]
    
    recv_data=b''
    
    if (prediction_label(pred) == 'OFF'):
        response = 'N'
    elif (prediction_label(pred) == 'ON'):
        response = 'Y'
    s.sendto(response.encode(),addr)


s.close()