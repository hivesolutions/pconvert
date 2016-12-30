#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import setuptools

setuptools.setup(
    name = "pconvert-python",
    version = "0.2.23",
    author = "Hive Solutions Lda.",
    author_email = "development@hive.pt",
    description = "PNG Converter",
    license = "Apache License, Version 2.0",
    keywords = "pconvert converted compositor",
    url = "http://pconvert.hive.pt",
    ext_modules = [
        setuptools.Extension(
            "pconvert",
            include_dirs = ["src/pconvert", "/usr/local/include"],
            libraries = [] if os.name in ("nt",) else ["m", "png"],
            library_dirs = ["/usr/local/lib"],
            extra_compile_args = [] if os.name in ("nt",) else [
                "-O2",
                "-finline-functions",
                "-Winline"
            ],
            sources = [
                "src/pconvert/extension.c",
                "src/pconvert/pconvert.c",
                "src/pconvert/stdafx.c",
                "src/pconvert/structs.c",
                "src/pconvert/util.c"
            ],
            define_macros = [
               ("PASS_ERROR", None)
            ]
        )
    ],
    classifiers = [
        "Development Status :: 5 - Production/Stable",
        "Topic :: Utilities",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        "Programming Language :: Python",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3.0",
        "Programming Language :: Python :: 3.1",
        "Programming Language :: Python :: 3.2",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4"
    ]
)
