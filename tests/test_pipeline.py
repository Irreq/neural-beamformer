import unittest

from build.lib import pipeline
from build import config

import numpy as np

class Pipeline(unittest.TestCase):
    """Tests that pipeline works as expected when inserting and reading items"""

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

    def test_circular_all_delay(self):
        """Tests that circular delay works as expected with multiple frames and wrap-around
        
        NOTICE this tests that if we have linear incrementation can be delayed properly
        anything else is difficult to measure, and must be assumed to work
        based on the result of this test.

        For example, a sine wave can easely be measured with delay, its only the phase. 
        But this does not work for noise.

        """
  
        pipe = pipeline.Pipeline()

        frames = np.zeros((config.N_SENSORS, config.N_SAMPLES), dtype=np.float32)

        # Fill the pipeline with dummy data
        for i in range(config.N + 1):
            tmp = frames[:] + np.arange(config.N_SAMPLES*i, config.N_SAMPLES*(i+1), dtype=np.float32)

            pipe.store_all(tmp)

    
        delays = np.arange(config.N_SENSORS, dtype=np.float32) / 2
        
        # we assume the delay difference should be uniform so we tile the delays
        expected_diffs = np.tile(delays[:, np.newaxis], (1, config.N_SAMPLES))

        # Retrieve the last frame as base
        last_frame = pipe.get_last_frame()

        # Perform a delay on the last frame
        delayed_last_frame = pipe.delay_last_frame(delays)

        # Measure the difference between the delay and the last frame
        # Since the frame is linear incremented, it should result in uniform diff
        diff = last_frame - delayed_last_frame
        
        # Since we are working with float precision, some differences must be tolerated
        self.assertTrue(np.allclose(diff, expected_diffs, rtol=1e-05, atol=1e-08))








 
        
