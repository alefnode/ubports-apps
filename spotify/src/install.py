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
from pathlib import Path
import tests


def first_start():
    os.system("touch /home/phablet/.cache/spotify.alefnode/survey.txt")
    data = Path('/home/phablet/.cache/spotify.alefnode/install.txt').read_text()
    if data is None:
        os.system("touch /home/phablet/.cache/spotify.alefnode/install.txt")
        os.system('ssh-keygen -f /home/phablet/.ssh/spotify-app -t rsa -N ""')
        os.system('cat /home/phablet/.ssh/spotify-app.pub >> /home/phablet/.ssh/authorized_keys')
        os.system("echo 1 > /home/phablet/.cache/spotify.alefnode/survey.txt")
    else:
        log = open('/home/phablet/.cache/spotify.alefnode/mopidy.log', 'w')
        result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "mopidy --config /mopidy.conf"'], stdout=log, stderr=log, shell=True)


def post_install():
    os.system('pub_key=$(cat /home/phablet/.ssh/spotify-app.pub) && sed -i \'/$pub_key/d\' /root/.ssh/authorized_keys')
    os.system('rm -rf /home/phablet/.ssh/spotify-app.pub && rm -rf /home/phablet/.ssh/spotify-app')

def libertine(command):
    # cmd = ['libertine-container-manager', 'create', '-i', 'spotify-app']
    #
    # with open('~/Documents/ps.txt', 'w') as out:
    #     createcontainer = subprocess.call(cmd, stdout=out)
    #createcontainer = os.system("libertine-container-manager create -i spotify-app")
    # os.system("libertine-container-manager exec -i spotify-app -c 'apt install -y curl'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'wget -q https://apt.mopidy.com/mopidy.gpg'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'apt-key add mopidy.gpg'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'wget -q -O /etc/apt/sources.list.d/mopidy.list https://apt.mopidy.com/stretch.list'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'apt update -y'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'apt install -y mopidy mopidy-spotify'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'apt install python-pip'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'pip install --upgrade setuptools'")
    # os.system("libertine-container-manager exec -i spotify-app -c 'pip install Mopidy-Iris'")
    #os.system(libertine-container-manager exec -i xenial -c "/bin/bash")
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            print(output)
    rc = process.poll()
    return rc

def install():
    container = tests.test_libertine()
    print(container)
    mopidy = tests.test_mopidy()
    print(mopidy)
    spotify = tests.test_spotify()
    print(spotify)
    if container == "../assets/loading.gif":
        print("Install Libertine")
        libertine_create()
    elif mopidy == "../assets/loading.gif":
        print("Install Mopidy")
        mopidy_install()
    elif spotify == "../assets/loading.gif":
        print("Install Spotify")
        spotify_install()
    result = "Done"
    print(result)
    return result

def libertine_create():
    log = open('/home/phablet/.cache/spotify.alefnode/install.txt', 'w')  # so that data written to it will be appended
    result = subprocess.Popen(['ssh -i /home/phablet/.ssh/spotify-app -o StrictHostKeyChecking=no localhost "libertine-container-manager create -i spotify-app -d xenial"'], stdout=log, stderr=log, shell=True)
    return result

def mopidy_install():
    log = open('/home/phablet/.cache/spotify.alefnode/install.txt', 'w')  # so that data written to it will be appended
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "apt install -y curl python-pip"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "wget -q -O /tmp/mopidy.gpg https://apt.mopidy.com/mopidy.gpg"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "apt-key add /tmp/mopidy.gpg"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "wget -q -O /etc/apt/sources.list.d/mopidy.list https://apt.mopidy.com/stretch.list"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "apt update -y"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "apt install -y mopidy mopidy-spotify"'], stdout=log, stderr=log, shell=True)
    result.wait()
    return result

def spotify_install():
    log = open('/home/phablet/.cache/spotify.alefnode/install.txt', 'w')  # so that data written to it will be appended
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "apt remove -y mopidy mopidy-spotify"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "pip uninstall mopidy mopidy-spotify"'], stdout=log, stderr=log, shell=True)
    result.wait()
    result = subprocess.Popen(['libertine-container-manager exec -i spotify-app -c "pip install setuptools==44.1.0 mopidy-spotify==3.1.0 mopidy-iris==3.39.0 mopidy==2.2.3"'], stdout=log, stderr=log, shell=True)
    result.wait()
    return result


def log_file():
    data = Path('/home/phablet/.cache/spotify.alefnode/install.txt').read_text()
    return data

def avoid_suspension():
    appsnotsuspend = subprocess.run(['gsettings get com.canonical.qtmir lifecycle-exempt-appids | awk -F "\'|\'" \'{print $2}\''], universal_newlines=True, shell=True, stdout=subprocess.PIPE)
    addnewapp = appsnotsuspend.stdout.splitlines()
    addnewapp.append("spotify.alefnode")
    addnewapp = str(addnewapp)
    print(addnewapp)
    os.system('gsettings set com.canonical.qtmir lifecycle-exempt-appids "' + addnewapp + '"')
