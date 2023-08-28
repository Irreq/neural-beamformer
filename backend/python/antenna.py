#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# File name: python/antenna.py
# Description: File for creating/moving/steering/plotting antennas
# Author: irreq (irreq@protonmail.com)
# Date: 26/08/2023

import numpy as np
np.set_printoptions(linewidth=120)

import matplotlib.pyplot as plt

from scipy.spatial.transform import Rotation as R


ROWS = 8
COLUMNS = 8

DISTANCE = 0.02  # Distance between sensor elements

SAMPLE_RATE = 48828.0  # Sampling rate
PROPAGATION_SPEED = 340.0  # Speed of sound in air

ARRAY_SEPARATION = 0  # Distance between arrays


SKIP_N_MICS = 2  # How many mics to skip (use every SKIP_N_MICS)


# N_ARRAYS = 2
# N_SENSORS = N_ARRAYS * ROWS * COLUMNS



ORIGO = np.zeros(3)


def find_middle(antenna):
    """Find the middle of an antenna constallation

    This function requires a homogenius antenna structure

    Args:
        antenna (_type_): The antenna

    Returns:
        np.ndarray[1]: (X, Y, Z)
    """

    return np.mean(antenna, axis=0)


def place_antenna(antenna, position):
    """Place antenna with its center of gravity at 'position'

    Args:
        antenna (_type_): The antenna to be moved
        position (np.ndarray[1]): (X, Y, Z)

    Returns:
        moved_antenna: the moved antenna to the new position
    """

    middle = find_middle(antenna)
    return antenna - middle + position


def create_antenna(position: np.ndarray[1] = ORIGO):
    """Create an antenna consisting of a collection of 3D coordinates

    Args:
        position (np.ndarray[1], optional): Position of the array. Defaults to ORIGO.

    Returns:
        Array: The created antenna
    """

    half = DISTANCE / 2
    antenna = np.zeros((COLUMNS * ROWS, 3))
    index = 0

    for row in range(ROWS):
        for col in range(COLUMNS):
            
            antenna[index, 0] = col * DISTANCE - COLUMNS * half + half

            antenna[index, 1] = row * DISTANCE - ROWS * half + half

            index += 1

    return place_antenna(antenna, position)


def plot_antenna(antenna, adaptive=[], inline=False, relative=False):
    """Plot the antenna in 3D space

    Args:
        antenna (_type_): _description_
        adaptive (list, optional): _description_. Defaults to [].
        inline (bool, optional): _description_. Defaults to False.
        relative (bool, optional): _description_. Defaults to False.
    """
    # if inline:
    #     %matplotlib inline
    # else:
    #     %matplotlib qt
    
    fig = plt.figure()
    ax = fig.add_subplot(projection="3d")

    a = antenna.copy()
    if relative:
        a[:,2] = a[:,2] - a[:,2].min()
        
    active = []
    inactive = []
    for i in range(a.shape[0]):
        if i in adaptive:
            active.append(i)
        else:
            inactive.append(i)

    ax.scatter(a[active,2], a[active,0], a[active,1], c="red", marker="o")
    ax.scatter(a[inactive,2], a[inactive,0], a[inactive,1], c="white", marker="o", edgecolors="black", linewidths=1,)

    plt.axis("equal")
    plt.set_cmap("jet")

    plt.show()


def compute_delays(antenna):
    """Compute the delays as the Z-value of the array

    Args:
        antenna (_type_): _description_

    Returns:
        np.ndarray[3]: _description_
    """
    delays = antenna[:,2] * (SAMPLE_RATE / PROPAGATION_SPEED)
    delays -= delays.min()

    return delays.reshape((-1, COLUMNS, ROWS))


def steer_center(antenna, azimuth: float, elevation: float):
    """Steer an antenna by pitch and yaw

    Args:
        antenna (_type_): The antenna of 3D points to be steered
        azimuth (float): Angle X-axis 
        elevation (float): Angle Y-axis

    Returns:
        np.ndarray[2]: The steered array
    """

    middle = find_middle(antenna)

    rotation = R.from_euler("xy", [np.radians(-elevation), np.radians(-azimuth)], degrees=False)
    
    # Rotate around origo
    rotated = rotation.apply(antenna - middle)

    # Move back into original position
    return rotated + middle


def used_sensors(antenna, distance: float=DISTANCE):
    """Calculate index of sensors to be used

    Args:
        antenna (np.ndarray[2]): The antenna
        distance (float, optional): The skip distance between microphones. Defaults to DISTANCE.

    Returns:
        list: indexes to be used
    """
    

    assert distance > 0, "Distance must be greater than 0"

    # Normalize antenna such that each position converts to a whole integer
    normalized_antenna = antenna / distance
    normalized_antenna = np.subtract(normalized_antenna, np.min(normalized_antenna, axis=0))
    normalized_antenna = np.round(normalized_antenna, 0)

    adaptive = []

    i = 0
    for (x, y, z) in normalized_antenna:

        if SKIP_N_MICS != 0:
            if x % SKIP_N_MICS == 0 and y % SKIP_N_MICS == 0:
                adaptive.append(i)
        else:
            adaptive.append(i)

        i += 1

    return adaptive


def create_combined_array(definition: list[list[int]], position: np.ndarray[1]=ORIGO):
    """Create a 2D array superstructure of smaller arrays

    Args:
        definition (list[list[int]]): How the arrays should be positioned
        position (np.ndarray[1], optional): Where the arrays will be located. Defaults to ORIGO.

    Returns:
        _type_: The new array
    """
    antennas = []

    SEPARATION_X = COLUMNS * DISTANCE + ARRAY_SEPARATION / 2
    SEPARATION_Y = ROWS * DISTANCE + ARRAY_SEPARATION / 2

    for i in range(len(definition)):
        for j in range(len(definition[i])):
            if definition[i][j]:
                antenna = create_antenna(np.array([SEPARATION_X * j, SEPARATION_Y * i,0]))
                antennas.append(antenna)
    
    # Create a final big array
    combined_array = np.concatenate(antennas)

    return place_antenna(combined_array, position)



if __name__ == "__main__":
    merged = create_combined_array([[1, 1, 1]])


    merged = create_combined_array([[1,0, 1]])
    # merged = create_antenna()
    adaptive = used_sensors(merged)
    final = place_antenna(merged, np.array([0, 0, 0]))

    final = steer_center(final, 45, 45)

    plot_antenna(final, adaptive=adaptive, relative=True)
    # plot_antenna(merged, adaptive=adaptive, relative=True)

    h = compute_delays(final)

    print(h.shape, h, np.max(h))
