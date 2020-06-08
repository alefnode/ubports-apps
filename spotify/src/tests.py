'''
 Copyright (C) 2020  Adrian Campos

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 3.

 ubuntu-calculator-app is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''

import os
import subprocess
import shlex
import time
from pathlib import Path

def test_call():
    result = subprocess.run(['ls -la /home/hadrianweb/ | grep aaa'], universal_newlines=True, stdout=subprocess.PIPE, shell=True)
    print(result)
    if result.stdout:
        result = "../assets/done.gif"
    else:
        result = "../assets/loading.gif"
    return result

def test_libertine():
    result = subprocess.run(['libertine-container-manager list | grep spotify-app'], universal_newlines=True, stdout=subprocess.PIPE, shell=True)
    if result.stdout:
        file_modify_epoch = os.path.getmtime("/home/phablet/.cache/spotify.alefnode/install.txt")
        actual_epoch = time.time()
        diff = actual_epoch - file_modify_epoch
        if diff <= 300:
            print("Escribiendo en el fichero", diff)
        else:
            print("Hora de testear el container", diff)
            result = subprocess.run(['libertine-container-manager exec -i spotify-app -c "echo Test"'], universal_newlines=True, stdout=subprocess.PIPE, shell=True)
            if result.stdout:
                result = "../assets/done.gif"
    else:
        result = "../assets/loading.gif"
    return result

def test_mopidy():
    container = test_libertine()
    if container == "../assets/done.gif":
        result = subprocess.run(['libertine-container-manager exec -i spotify-app -c "apt list --installed"'], universal_newlines=True, stdout=subprocess.PIPE, shell=True)
        if "mopidy" in result.stdout:
            result = "../assets/done.gif"
    else:
        result = "../assets/loading.gif"
    return result

def test_spotify():
    mopidy = test_libertine()
    if mopidy == "../assets/done.gif":
        result = subprocess.run(['libertine-container-manager exec -i spotify-app -c "find /usr/local/lib/python2.7/dist-packages/ -maxdepth 1 -type d -name \"mopidy_iris\""'], universal_newlines=True, stdout=subprocess.PIPE, shell=True)
        print(result.stdout)
        if result.stdout:
            result = "../assets/done.gif"
    else:
        result = "../assets/loading.gif"
    print(result)
    return result
