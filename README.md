# P(NG)Convert

Simple PNG conversion tool using libpng.

## Build

It should be simple to build the project using `make`. Please note that the project requires both Python and libpng.

```bash
make
make install
```

To build pconvert for a specific system use the `SYS` Makefile variable:

```bash
make SYS=darwin
```

It's also possible to build the Python extension and install in a local machine.

```bash
python setup.py install
```

## Example

```bash
pconvert compose assets/demo/
```

```bash
pconvert convert assets/demo/tux.png tux.out.png
```

## License

PConvert is currently licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/).

## Build Automation

[![Build Status](https://travis-ci.org/hivesolutions/pconvert.svg?branch=master)](https://travis-ci.org/hivesolutions/pconvert)
[![Coverage Status](https://coveralls.io/repos/hivesolutions/pconvert/badge.svg?branch=master)](https://coveralls.io/r/hivesolutions/pconvert?branch=master)
[![PyPi Status](https://img.shields.io/pypi/v/pconvert-python.svg)](https://pypi.python.org/pypi/pconvert-python)
