import cv2
import numpy as np
import socket

import matplotlib.pyplot as plt

def generate_color_map(name="jet") -> np.ndarray:
    """Create a color lookup table for values between 0 - 255

    Args:
        name (str, optional): Matplotlib CMap. Defaults to "jet".

    Returns:
        np.ndarray: the lookup-table
    """
    
    cmap = plt.cm.get_cmap(name)

    # Generate color lookup table
    colors = np.empty((256, 3), dtype=np.uint8)

    for i in range(256):
        colors[i] = (np.array(cmap(255 - i)[:3]) * 255).astype(np.uint8)

    return colors


colors = generate_color_map()

WINDOW_DIMENSIONS = (1000, 1000)
POWER = 1

def calculate_heatmap(image, threshold=1e-7, amount = 0.5, exponent = POWER):
    """Create a heatmap over the perceived powerlevel

    Args:
        image (np.ndarray[MAX_RES_X, MAX_RES_Y]): The calculated powerlevels for each anlge
        threshold (float, optional): minimum max value to print out. Defaults to 5e-8.

    Returns:
        (heatmap, bool): the calculated heatmap and if it should be output or not
    """
    MAX_RES_X, MAX_RES_Y = image.shape
    should_overlay = False
    small_heatmap = np.zeros((MAX_RES_Y, MAX_RES_X, 3), dtype=np.uint8)
    
    max_power_level = np.max(image)

    

    if max_power_level > threshold:

        img = np.log10(image)
        img -= np.log10(np.min(image))
        img /= np.max(img)
        # img = 1 - image
        # img **=10


        should_overlay = True
        # Convert image value in range between [0, 1] to a RGB color value
        for x in range(MAX_RES_X):
            for y in range(MAX_RES_Y):
                power_level = img[x, y]

                # Only paint levels above a certain amount, i.e 50%
                if power_level >= amount:
                    power_level -= amount
                    power_level /= amount

                    # Some heatmaps are very flat, so the power of the power
                    # May give more sharper results
                    color_val = int(255 * power_level ** exponent)

                    # This indexing is a bit strange, but CV2 orders it like this (Same as flip operation)
                    small_heatmap[MAX_RES_Y - 1 - y, MAX_RES_X - 1 - x] = colors[color_val]
    # Must resize to fit camera dimensions
    heatmap = cv2.resize(small_heatmap, WINDOW_DIMENSIONS, interpolation=cv2.INTER_LINEAR)
    
    return heatmap, should_overlay


# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind(("0.0.0.0", 54321))  # Specify the port to listen on

height = 20  # Adjust the dimensions as needed
width = height
heatmap_data_size = width*height  # Size of the heatmap_data array

prev = np.zeros((1000, 1000, 3), dtype=np.uint8)
while True:
    # Receive UDP packet
    data, _ = udp_socket.recvfrom(heatmap_data_size * 4)  # 4 bytes per float32
    
    heatmap_data = np.frombuffer(data, dtype=np.float32).reshape((height, width))  # Convert received data to a NumPy array
    # print(heatmap_data)
    t = heatmap_data.copy()
    

    #print(t.mean())

    t, draw = calculate_heatmap(t, threshold=5e-8)
    frame = np.zeros_like(t)

    image = cv2.addWeighted(prev, 0.5, t, 0.5, 0)
    prev = t
    if not draw:
        image = frame

    # Display the heatmap using OpenCV
    cv2.imshow('Heatmap', image)
    cv2.waitKey(1)  # Adjust the delay as needed
