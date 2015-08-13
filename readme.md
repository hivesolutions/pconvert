# P(NG)Convert

Simple PNG conversion tool using libpng.

## Build

It should be simple to build the project using `make`. Please note that the project requires both python and libpng.

```bash
make
make install
```

It's also possible to build the python extension and install in a local machine.

```bash
python setup.py install
```

## Example

```bash
pconvert assets/demo/tux.png tux.out.png
```

## License

PConvert is currently licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/).

## Build Automation

[![Build Status](https://travis-ci.org/joamag/pconvert.svg?branch=master)](https://travis-ci.org/joamag/pconvert)
[![Coverage Status](https://coveralls.io/repos/joamag/pconvert/badge.svg?branch=master)](https://coveralls.io/r/joamag/pconvert?branch=master)
[![PyPi Status](https://img.shields.io/pypi/v/pconvert-python.svg)](https://pypi.python.org/pypi/pconvert-python)
