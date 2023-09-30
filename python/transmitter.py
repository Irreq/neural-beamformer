#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket
import struct
import time
import numpy as np

from config import *

"""
Simple script to emulate a data transfer from the antenna
"""

# Define the server address and port
server_address = (UDP_ADDRESS, UDP_PORT)

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


# Actual transmission protocol see receiver.h
TEST_FREQUENCY = 100 #48828 # uint16
TEST_N_ARRAYS = 4 #int8
TEST_PROTOCOL_VERSION = 1 #int8
TEST_COUNTER = 0 #int32

PAYLOAD_DTYPE = np.int32

int32_min = -2147483648  # Minimum possible int32 value
int32_max = 2147483647   # Maximum possible int32 value

def denormalize_int32(normalized_value) -> int:
    if normalized_value < 0:
        return int(-normalized_value * int32_min)
    else:
        return int(normalized_value * int32_max)

def generate_data():
    pos = 0
    while True:
        yield np.arange(pos*N_SENSORS, (pos+1)*N_SENSORS, dtype=np.float32) 
        pos += 1

def sine_stream_generator(source: float = TEST_FREQUENCY / 10, amplitude=1.0):
    t = 0
    while True:
        sine = np.sin(2 * np.pi * source * t) * amplitude
        # sine = ((t*TEST_FREQUENCY) % TEST_FREQUENCY) / TEST_FREQUENCY
        # print(sine)

        yield np.ones(N_SENSORS, dtype=PAYLOAD_DTYPE) * denormalize_int32(sine)
        t += 1 / TEST_FREQUENCY

generator = sine_stream_generator()
counter = 0

while True:
    try:
        # Pack the int8 and the float array into separate binary representations
        header = struct.pack('Hbbi',
                             TEST_FREQUENCY,
                             TEST_N_ARRAYS,
                             TEST_PROTOCOL_VERSION,
                             counter)

        data = next(generator)
        
        # payload as C-style float32 byte array
        payload = data.astype(PAYLOAD_DTYPE).tobytes(order='C')

        # Combine the binary data into a single message
        msg = header + payload

        # Send the data to the server
        sock.sendto(msg, server_address)

        # Wait for 1 second before sending the next array
        time.sleep(1 / TEST_FREQUENCY)

        counter += 1

    except:
        break

# Close the socket (this part is never reached in this example)
sock.close()

