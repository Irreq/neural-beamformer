import unittest

from build.lib import pipeline
from build import config

import numpy as np

class Pipeline(unittest.TestCase): 

    def test_creation(self):
        """Tests that an actual pipeline is being created"""
        pipe = pipeline.Pipeline()
        self.assertTrue(isinstance(pipe, pipeline.Pipeline))

    def test_no_noise_creation(self):
        """Tests that when a frame is crated it will be zeroed"""
        pipe = pipeline.Pipeline()
        truth = np.zeros((config.N_SENSORS, config.BUFFER_LENGTH), dtype=np.float32)
        self.assertTrue(np.array_equal(pipe.latest(), truth))

    def test_insertion(self):
        """Tests that when inserting a frame, it should be positioned at the end"""
        pipe = pipeline.Pipeline()

        frame = np.ones(config.N_SENSORS, dtype=np.float32)

        pipe.store(frame)

        truth = np.zeros((config.N_SENSORS, config.BUFFER_LENGTH), dtype=np.float32)
        truth[:,-1] = 1.0

        self.assertTrue(np.array_equal(pipe.latest(), truth))

    def test_circular_insertion(self):
        """Tests that circular insertion works as expected with wrap-around"""
        pipe = pipeline.Pipeline()

        frame = np.zeros(config.N_SENSORS, dtype=np.float32)

        for i in range(config.BUFFER_LENGTH + 1):
            pipe.store(frame + i)

        truth = np.arange(config.BUFFER_LENGTH, dtype=np.float32) 
        truth = np.tile(truth, (config.N_SENSORS, 1))

        self.assertTrue(np.array_equal(pipe.latest(), truth + 1))

    def test_all_insertion(self):
        """Tests that when multiple frames are added that they are added to the end"""
        pipe = pipeline.Pipeline()

        frames = np.zeros((config.N_SENSORS, config.N_SAMPLES), dtype=np.float32)

        frames[:] = np.arange(config.N_SAMPLES, dtype=np.float32)

        pipe.store_all(frames)

        truth = np.zeros((config.N_SENSORS, config.BUFFER_LENGTH), dtype=np.float32)
        truth[:,-config.N_SAMPLES:] = np.arange(config.N_SAMPLES, dtype=np.float32)

        self.assertTrue(np.array_equal(pipe.latest(), truth))

    def test_circular_all_insertion(self):
        """Tests that circular insertion works as expected with multiple frames and wrap-around"""
  
        pipe = pipeline.Pipeline()

        frames = np.zeros((config.N_SENSORS, config.N_SAMPLES), dtype=np.float32)

        for i in range(config.N + 1):
            pipe.store_all(frames[:] + np.arange(config.N_SAMPLES*i, config.N_SAMPLES*(i+1), dtype=np.float32))

        truth = np.zeros((config.N_SENSORS, config.BUFFER_LENGTH), dtype=np.float32)
        truth[:] = np.arange(config.N_SAMPLES, config.BUFFER_LENGTH + config.N_SAMPLES, dtype=np.float32)

        self.assertTrue(np.array_equal(pipe.latest(), truth))







 
        
