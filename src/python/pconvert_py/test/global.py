#!/usr/bin/python
# -*- coding: utf-8 -*-

import unittest

import pconvert

class GlobalTest(unittest.TestCase):

    def test_basic(self):
        self.assertEqual(type(pconvert.VERSION), str)
        self.assertEqual(pconvert.VERSION, "0.3.11")

        self.assertEqual(type(pconvert.FEATURES), str)

        self.assertEqual(type(pconvert.EXTENSION), bool)
        self.assertEqual(pconvert.EXTENSION, True)

        self.assertEqual(type(pconvert.OPENCL), bool)
