#!/usr/bin/python
# -*- coding: utf-8 -*-

import unittest

import pconvert

class GlobalTest(unittest.TestCase):

    def test_basic(self):
        self.assertEqual(type(pconvert.VERSION), str)
        self.assertEqual(pconvert.VERSION, "0.3.10")
