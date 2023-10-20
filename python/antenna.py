#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# File name: python/antenna.py
# Description: File for creating/moving/steering/plotting antennas
# Author: irreq (irreq@protonmail.com)
# Date: 26/08/2023

import numpy as np
np.set_printoptions(linewidth=120)

import matplotlib.pyplot as plt

from config import *


# ROWS = 8
# COLUMNS = 8

# DISTANCE = 0.02  # Distance between sensor elements

# SAMPLE_RATE = 48828.0  # Sampling rate
PROPAGATION_SPEED = 340.0  # Speed of sound in air

ARRAY_SEPARATION =2* DISTANCE #0 # DISTANCE / 2  # Distance between arrays


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

    #ax.axis('') #this line fits your images to screen 
    plt.axis("equal")
    plt.set_cmap("jet")

    ax.set_yticklabels([])
    ax.set_xticklabels([])
    ax.set_zticklabels([])

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

    return delays.reshape((-1, ROWS, COLUMNS))


def compute_rotation_matrix(azimuth: float, elevation: float, degrees=True):
    if degrees:
        azimuth = np.radians(azimuth)
        elevation = np.radians(elevation)
    rotation_matrix_yaw = np.array([
        [np.cos(azimuth), 0, np.sin(azimuth)],
        [0, 1, 0],
        [-np.sin(azimuth), 0, np.cos(azimuth)]
    ])

    rotation_matrix_pitch = np.array([
        [1, 0, 0],
        [0, np.cos(elevation), -np.sin(elevation)],
        [0, np.sin(elevation), np.cos(elevation)]
    ])

    combined_rotation_matrix = rotation_matrix_yaw @ rotation_matrix_pitch

    return combined_rotation_matrix.T



def steer_center(antenna, azimuth: float, elevation: float):
    """Steer an antenna by pitch and yaw

    TODO: Change from scipy implementation to C for speedup

    Args:
        antenna (_type_): The antenna of 3D points to be steered
        azimuth (float): Angle X-axis 
        elevation (float): Angle Y-axis

    Returns:
        np.ndarray[2]: The steered array
    """

    middle = find_middle(antenna)

    rotated = (antenna - middle) #@ compute_rotation_matrix(np.radians(-azimuth), np.radians(-elevation), degrees=False)

    rotated @= compute_rotation_matrix(np.radians(-azimuth), np.radians(-elevation), degrees=False)
    
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


def generate_angles_sphere(n: int):
    """Create n angles on a unitsphere

    Args:
        n (int): number of points to create

    Returns:
        (float, float): polar angles to each point
    """
    i = np.arange(0, n, dtype=np.float32) + 0.5
    phi = np.arccos(1 - 2*i/n)
    golden_ratio = (1 + 5**0.5)/2

    theta = 2 * np.pi * i / golden_ratio

    return theta, phi


def generate_points_half_sphere(n: int):
    """Create n points on a dome

    Args:
        n (int): number of points

    Returns:
        np.ndarray: points -> (n, 3)
    """

    theta, phi = generate_angles_sphere(2*n)

    valid = (np.pi / 2) >= phi


    theta = theta[valid]
    phi = phi[valid]

    x = np.cos(theta) * np.sin(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(phi)

    points = np.vstack((x, y, z))

    return points.T


def convert_points_to_polar_angles(points: np.ndarray):
    """Converts points to polar angles

    Args:
        points (np.ndarray): the points that will be converted

    Returns:
        (np.ndarray): polar angles -> (n, 2)
    """
    x = points[:,0]
    y = points[:,1]
    z = points[:,2]


    r = np.sqrt(x**2 + y**2 + z**2)
    phi = np.arccos(z / r)
    theta = np.arctan2(y, x)

    return np.vstack((theta, phi)).T


def convert_points_to_steering_angles(points: np.ndarray):
    """Converts points to steering angles

    Args:
        points (np.ndarray): the points that will be converted

    Returns:
        (np.ndarray): steering angles -> (n, 2)
    """
    x = points[:,0]
    y = points[:,1]
    z = points[:,2]

    azimuth = np.degrees(np.arctan2(z, y)  - np.pi/2)
    elevation = np.degrees(np.arctan2(z, x) - np.pi/2)

    return np.vstack((azimuth, elevation)).T


if __name__ == "__main__":

    points = generate_points_half_sphere(50*50)

    # points.append([0, 0, 1.1])

    p = np.array([0, 0, 1.03])

    points = np.vstack((points, p))



    plot_antenna(points, relative=True)

    exit()

    merged = create_combined_array([[1, 1, 1]])
    final = place_antenna(merged, np.array([0, 0, 0]))

    # merged = create_antenna()
    adaptive = used_sensors(final)
    
    final = steer_center(final, 30, 10)

    h = compute_delays(final)

    print(h.shape, np.max(h))
    print(np.round(h, 2), "Delays")

    plot_antenna(final, adaptive=adaptive, relative=True)
    # plot_antenna(merged, adaptive=adaptive, relative=True)

    
