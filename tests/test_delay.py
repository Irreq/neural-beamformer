
import unittest

from build.lib import delay

class Delay(unittest.TestCase):
    def test_burg(self):
        self.assertEqual(delay.get_version(), "0")

    def test(self):
        """Testing random gibberish"""
        # result = delay.test()
        # print(result)

