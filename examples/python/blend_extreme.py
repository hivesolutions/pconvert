#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

import pconvert

SIZE = 1000000

for i in range(SIZE):
    pconvert.blend_multiple(
        (
            os.path.abspath("../../assets/demo/sole.png"),
            os.path.abspath("../../assets/demo/back.png"),
            os.path.abspath("../../assets/demo/front.png"),
        ),
        os.path.abspath("result.extreme.png"),
    )

    pconvert.blend_multiple(
        (
            os.path.abspath("../../assets/demo/sole.png"),
            os.path.abspath("../../assets/demo/back.png"),
            os.path.abspath("../../assets/demo/front.png"),
        ),
        os.path.abspath("result.extreme_destination_over.png"),
        algorithm="destination_over",
        is_inline=True,
    )

    if i % 100 == 0:
        print(i)
