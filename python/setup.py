import platform
import os
import subprocess
import sys
from shutil import copyfile, rmtree
import distutils.log as dlog
from distutils.command.build import build
from setuptools import setup, find_packages

if sys.version_info[:2] < (3, 5):
    raise RuntimeError("Python version >= 3.5 required.")

here = os.path.abspath(os.path.dirname(__file__))
if os.path.isfile(os.path.join(here, "version")):
    version_file = os.path.join(here, "version")
elif os.path.isfile(os.path.join(here, "rdrive", "version")):
    version_file = os.path.join(here, "rdrive", "version")
else:
    raise FileNotFoundError("Version file was not found")

NAME = "rdrive"
DESCRIPTION = "Python API for RDrive servomotors"
URL = "https://rozum.com"
EMAIL = "dev@rozum.com"
AUTHOR = "Rozum Robotics"
with open(version_file) as v:
    VERSION = v.readline().rstrip()
REQUIRED = ["Deprecated==1.2.6"]
DEPENDENCY_LINKS = []

if "dev" in VERSION:
    DEVELOPMENT_STATUS = "Development Status :: 4 - Beta"
else:
    DEVELOPMENT_STATUS = "Development Status :: 5 - Production/Stable"

CLASSIFIERS = [
    "License :: OSI Approved :: MIT License",
    DEVELOPMENT_STATUS,
    "Programming Language :: Python",
    "Programming Language :: Python :: 3",
    "Programming Language :: Python :: Implementation :: CPython",
    "Operating System :: MacOS",
    "Operating System :: Microsoft :: Windows",
    "Operating System :: POSIX :: Linux",
]

try:
    with open(os.path.join(here, "README.md"), encoding="utf-8") as f:
        LONG_DESCRIPTION = "\n" + f.read()
except FileNotFoundError:
    LONG_DESCRIPTION = DESCRIPTION


class Build(build):
    __LIB_UNIX = "libservo_api.so"
    __LIB_WIN = "libservo_api-{}.dll"

    def run(self):
        if os.name == "nt":
            bit_v = platform.architecture()[0]
            self.announce(
                "Installing prebuilt {} windows library".format(bit_v),
                dlog.INFO,
            )
            lib_file = self.__LIB_WIN.format(bit_v)
            source_path = os.path.join(here, "rdrive/lib/{}".format(lib_file))
            assert os.path.exists(
                source_path
            ), "Windows source path does not exist"
            target_path = os.path.join(
                here, "rdrive/servo/{}".format(lib_file)
            )
            copyfile(source_path, target_path)
            self.announce("Windows library successfully installed", dlog.INFO)
        else:
            self.announce("Building C library", dlog.INFO)
            make_cmd = ["make"]
            make_clean_cmd = ["make", "clean"]
            if subprocess.call(make_cmd, cwd="rdrive") != 0:
                sys.exit(-1)
            lib_file = self.__LIB_UNIX
            source_path = os.path.join(
                here, "rdrive/build/{}".format(lib_file)
            )
            assert os.path.exists(source_path), "Build path does not exist"
            target_path = os.path.join(
                here, "rdrive/servo/{}".format(lib_file)
            )
            copyfile(source_path, target_path)
            if subprocess.call(make_clean_cmd, cwd="rdrive") != 0:
                sys.exit(-1)
        self.announce("Removing unnecessary sources", dlog.INFO)
        rmtree(os.path.join(here, "rdrive/include"))
        rmtree(os.path.join(here, "rdrive/src"))
        rmtree(os.path.join(here, "rdrive/lib"))
        os.remove(os.path.join(here, "rdrive/core.mk"))
        os.remove(os.path.join(here, "rdrive/Makefile"))
        build.run(self)


setup(
    name=NAME,
    version=VERSION,
    packages=find_packages(exclude=("examples")),
    install_requires=REQUIRED,
    dependency_links=DEPENDENCY_LINKS,
    url=URL,
    license="MIT",
    classifiers=CLASSIFIERS,
    author=AUTHOR,
    author_email=EMAIL,
    description=DESCRIPTION,
    long_description=LONG_DESCRIPTION,
    cmdclass={"build": Build,},
    package_data={"": ["*.so", "*.dll"]},
    zip_safe=False,
)
