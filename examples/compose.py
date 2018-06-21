#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

import pconvert

pconvert.blend_images(
    os.path.abspath("../assets/demo/sole.png"),
    os.path.abspath("../assets/demo/back.png"),
    os.path.abspath("result.png")
)

pconvert.blend_images(
    os.path.abspath("result.png"),
    os.path.abspath("../assets/demo/front.png"),
    os.path.abspath("result.png")
)
