#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

import pconvert

pconvert.blend_images(
    os.path.abspath("../../assets/demo/sole.png"),
    os.path.abspath("../../assets/demo/back.png"),
    os.path.abspath("result.png")
)

# this example supplies an optional parameter called `options` which pconvert-rust uses
# despite not being used by this pconvert version, it is backwards compatible
pconvert.blend_images(
    os.path.abspath("result.png"),
    os.path.abspath("../../assets/demo/front.png"),
    os.path.abspath("result.png"),
    options = {
        "num_threads": 20,
        "compression": "best",
        "filter": "nofilter"
    }
)
