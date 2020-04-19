# [P(NG)Convert](http://pconvert.hive.pt)

Simple PNG conversion tool using [libpng](http://www.libpng.org).

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

## Python Methods

### `blend_images`

| Argument | Type | Mandatory | Default | Description |
| --- | --- | --- | --- | --- |
| bottom_path | `str` | `true` | - | The path to the bottom image. |
| top_path | `str` | `true` | - | The path to the top image. |
| target_path | `str` | `true` | - | The path of the target (resulting) image. |
| algorithm | `str` | `false` | `multiplicative` | The blending algorithm to be used. |
| is_inline | `bool` | `false` | `false` | If the inline methods should be used to perform the blending. |

### `blend_multiple`

| Argument | Type | Mandatory | Default | Description |
| --- | --- | --- | --- | --- |
| paths | `list` | `true` | - | The sequence of paths of the images to be blended from bottom to top. |
| target_path | `str` | `true` | - | The path of the target (resulting) image. |

## Example

Running a simple set of composition can be done using:

```bash
pconvert compose assets/demo/
```

To be able to convert one image into a normalized PNG format use the `convert` command as following:

```bash
pconvert convert assets/demo/tux.png tux.out.png
```

To run a simple benchmark operation (results in `benchmark.txt`) using pconvert use:

```bash
pconvert benchmark assets/demo/
```

## Conan

This package makes use of the [Conan](https://conan.io) package manager to use run:

```bash
pip install --upgrade conan
conan install . --build missing
```

It's important to note that the usage of Conan is not mandatory and it's still possible to build P(NG)Convert without it.

## CMake

### Windows

```bash
conan install . --build missing
cmake . -DCMAKE_CL_64=1 -DCMAKE_GENERATOR_PLATFORM=x64 -Ax64
msbuild ALL_BUILD.vcxproj /P:Configuration=Release
```

## License

PConvert is currently licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/).

## Build Automation

[![Build Status](https://travis-ci.org/hivesolutions/pconvert.svg?branch=master)](https://travis-ci.org/hivesolutions/pconvert)
[![Build Status GitHub](https://github.com/hivesolutions/pconvert/workflows/Main%20Workflow/badge.svg)](https://github.com/hivesolutions/pconvert/actions)
[![Coverage Status](https://coveralls.io/repos/hivesolutions/pconvert/badge.svg?branch=master)](https://coveralls.io/r/hivesolutions/pconvert?branch=master)
[![PyPi Status](https://img.shields.io/pypi/v/pconvert-python.svg)](https://pypi.python.org/pypi/pconvert-python)
