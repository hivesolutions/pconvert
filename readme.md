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
