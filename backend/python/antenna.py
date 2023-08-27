import numpy as np
np.set_printoptions(linewidth=120)
import matplotlib.pyplot as plt
from scipy.spatial.transform import Rotation as R

N_ARRAYS = 2
ROWS = 8
COLUMNS = 8

DISTANCE = 0.02

SAMPLE_RATE = 48828.0
PROPAGATION_SPEED = 340.0

ARRAY_SEPARATION = DISTANCE


SKIP_N_MICS = 2

# X = 10
# Y = 10

X = COLUMNS*DISTANCE#+ARRAY_SEPARATION
Y = ROWS*DISTANCE#+ARRAY_SEPARATION

N_SENSORS = N_ARRAYS * ROWS * COLUMNS

def create_antenna(position: np.ndarray[1] = np.zeros(3)):
    half = DISTANCE / 2
    antenna = np.zeros((COLUMNS*ROWS, 3))
    index = 0
    for row in range(ROWS):
        for col in range(COLUMNS):
            
            antenna[index, 0] = col * DISTANCE - COLUMNS * half + half

            antenna[index, 1] = row * DISTANCE - ROWS * half + half

            index += 1

    antenna += position

    return antenna

def steer(antenna, azimuth: float, elevation: float):
    rotation = R.from_euler("xy", [np.radians(-elevation), np.radians(-azimuth)], degrees=False)
    return rotation.apply(antenna)

def plot_antenna(antenna, adaptive=[], inline=False, relative=False):
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
    delays = antenna[:,2] * (SAMPLE_RATE/PROPAGATION_SPEED)
    delays -= delays.min()
    delays = delays.reshape((-1, COLUMNS, ROWS))

    return delays

def find_middle(antenna):
    cp = antenna.copy()

    return np.mean(cp, axis=0)


def test_steer(antenna, azimuth: float, elevation: float):
    cp = antenna.copy()

    middle = find_middle(cp)
    cd = cp - middle
    rotation = R.from_euler("xy", [np.radians(-elevation), np.radians(-azimuth)], degrees=False)
    r = rotation.apply(cd)
    return r + middle

def used_sensors(antenna):
    adaptive = []

    cp = antenna.copy()

    n = cp.shape[0] - 1

    cp = cp / DISTANCE

    cp = np.subtract(cp, np.min(cp, axis=0))
    cp = np.round(cp, 0)

    pos = 0
    for (x, y, z) in cp:
        print(x, y, z)
        if SKIP_N_MICS != 0:
            if x % SKIP_N_MICS == 0 and y % SKIP_N_MICS == 0:
                adaptive.append(pos)
        else:
            adaptive.append(pos)

        pos += 1

    return adaptive

def create_combined_array(definition):
    antennas = []
    for i in range(len(definition)):
        for j in range(len(definition[i])):
            if definition[i][j]:
                antenna = create_antenna(np.array([X*j, Y*i,0]))
                antennas.append(antenna)
    return np.concatenate(antennas)

def place_antenna(antenna, position):
    middle = find_middle(antenna)
    return antenna.copy() - middle + position

if __name__ == "__main__":
    # merged = create_combined_array([[1, 1, 1]])


    merged = create_combined_array([[1]])
    # merged = create_antenna()
    adaptive = used_sensors(merged)
    final = place_antenna(merged, np.array([0, 0, 0]))

    final = test_steer(final, 20, 20)

    plot_antenna(final, adaptive=adaptive, relative=True)