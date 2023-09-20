
import unittest

import numpy as np

import build.lib.antenna as module

import python.antenna as truth

from build.config import *

class Antenna(unittest.TestCase):
    """Tests that antenna works as expected from the Python implementation"""

    def test_creation(self):
        """Tests that an actual Array is being created"""
        origo = np.zeros(3)
        truth_antenna = truth.create_antenna(origo)

        module_antenna = module._create_antenna(origo)

        self.assertTrue(np.allclose(module_antenna, truth_antenna, rtol=1e-05, atol=1e-08))





    def test(self):
        """Tests that proper delay is being implemented"""
        module_delays = module.test()

        pos = np.array([0.0, 0.0, 0.0])
        other = truth.create_antenna(pos)
        steered = truth.steer_center(other, 10.0, 5.0)

        truth_delays = truth.compute_delays(steered)
        # result = np.zeros((COLUMNS, ROWS), dtype=np.float32)

        # i = 0
        # for y in range(ROWS):
        #     for x in range(COLUMNS):
        #         result[y, x] = delays[i]
        #         i += 1

        self.assertTrue(np.allclose(module_delays, truth_delays, rtol=1e-05, atol=1e-08))




