## CFLAGS='-arch i386' LDFLAGS='-arch i386' python setup.py build
## CFLAGS='-arch i386 -I/usr/local/include -L/usr/local/lib' LDFLAGS='-arch i386' python setup.py build

CFLAGS='-I/usr/local/include -L/usr/local/lib' python setup.py build

sudo python setup.py install

