#!/usr/bin/python
# -*- coding: utf-8 -*-

# Sets of tests related to MDN documentation @ https://developer.mozilla.org/docs/Web/API/Canvas_API/Tutorial/Compositing/Example

import os

import pconvert

pconvert.blend_images(
    os.path.abspath("../../assets/mozilla/source.png"),
    os.path.abspath("../../assets/mozilla/destination.png"),
    os.path.abspath("result.source_over.png"),
    "source_over"
)

pconvert.blend_images(
    os.path.abspath("../../assets/mozilla/source.png"),
    os.path.abspath("../../assets/mozilla/destination.png"),
    os.path.abspath("result.destination_over.png"),
    "destination_over"
)
