#!/usr/bin/env python3

import os
import subprocess

giomodulesdir = os.path.join(os.environ['MESON_INSTALL_PREFIX'], 'lib', 'gio',
                             'modules')

if not os.environ.get('DESTDIR'):
    print('Updating GIO modules')
    subprocess.call(['gio-querymodules', giomodulesdir])
