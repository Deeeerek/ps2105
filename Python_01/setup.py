#!/usr/bin/python
from distutils.core import setup, Extension

module1 = Extension('PicoScope',
                    #sources = ['PicoScope.c'],
                    sources = ['PicoScope.c', 'libPicoScope.c', 'libPicoScope-data.c'],
                    libraries=['usb-1.0'],
                    include_dirs=['.'],
                    extra_compile_args=['-std=c99'],
                    )

setup (name = 'PicoScope',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1],
       )
