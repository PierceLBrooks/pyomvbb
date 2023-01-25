#!/usr/bin/env python
# https://github.com/arvidn/libtorrent/blob/RC_2_0/bindings/python/setup.py + https://github.com/rmcgibbo/pypatchelf/blob/master/setup.py

import os
import sys
import glob
import site
import shutil
import inspect
import platform
import subprocess
import distutils.cmd
import distutils.command.install_data as install_data_lib
from distutils import log
from distutils.command.build_ext import build_ext
from setuptools import setup, Distribution, Extension, find_packages
from typing import cast
from typing import Iterator

def wd():
    filename = inspect.getframeinfo(inspect.currentframe()).filename
    path = os.path.dirname(os.path.abspath(filename))
    return path

# Frustratingly, the "bdist_*" unconditionally (re-)run "build" without
# args, even ignoring "build_*" earlier on the same command line. This
# means "build_*" must be a no-op if some build output exists, even if that
# output might have been generated with different args (like
# "--define=FOO"). b2 does not know how to be "naively idempotent" like
# this; it will only generate outputs that exactly match the build request.
#
# It doesn't work to short-circuit initialize_options() / finalize_options(),
# as this doesn't play well with the way options are externally manipulated by
# distutils.
#
# It DOES work to short-circuit Distribution.reinitialize_command(), so we do
# that here.
class ShortCircuitDistribution(Distribution):
    def reinitialize_command(
        self, command: str, reinit_subcommands: int = 0
    ) -> distutils.cmd.Command:
        if command == "build_ext":
            return cast(distutils.cmd.Command, self.get_command_obj("build_ext"))
        return cast(
            distutils.cmd.Command,
            super().reinitialize_command(
                command, reinit_subcommands=reinit_subcommands
            ),
        )
        
# Various setuptools logic expects us to provide Extension instances for each
# extension in the distro.
class StubExtension(Extension):
    def __init__(self, name: str):
        # An empty sources list ensures the base build_ext command won't build
        # anything
        sources = []
        #sources += glob.glob(os.path.join(wd(), "*.py"))
        super().__init__(name, sources=sources)
        
class InstallDataToLibDir(install_data_lib.install_data):
    def finalize_options(self) -> None:
        # install_data installs to the *base* directory, which is useless.
        # Nothing ever gets installed there, no tools search there. You could
        # only make use of it by manually picking the right install paths.
        # This instead defaults the "install_dir" option to be "install_lib",
        # which is "where packages are normally installed".
        self.set_undefined_options(
            "install",
            ("install_lib", "install_dir"),  # note "install_lib"
            ("root", "root"),
            ("force", "force"),
        )

class build_ext(build_ext):
    def run(self) -> None:
        self._build_extension()
        super().run()

    def _build_extension(self) -> None:
        packages = site.getsitepackages()[0].replace("\\", "/").split("/")
        system = platform.system()
        library_extension = ""
        script_extension = "sh"
        if (system == "Windows"):
            library_extension += "dll"
            script_extension = "bat"
        elif (system == "Linux"):
            library_extension += "so"
        elif (system == "Darwin"):
            library_extension += "dylib"
        for i in range(len(packages)):
            path = "/".join(packages[:(len(packages)-i)])
            if ((os.path.isdir(os.path.join(path, "lib"))) or (os.path.isdir(os.path.join(path, "Lib")))):
                packages = packages[:(len(packages)-i)]
                break
        script = os.path.join(wd(), "build."+script_extension)
        command = [script, system, platform.python_version(), "/".join(packages), sys.executable]
        log.info("execute \"%s\"", str(command))
        subprocess.run(command, cwd=wd(), check=True)
        src = os.path.join(wd(), "pyomvbb", "omvbb", "omvbb."+library_extension)
        dst = self.get_ext_fullpath(self.extensions[0].name)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        log.info("copy \"%s\" -> \"%s\"", src, dst)
        shutil.copyfile(src, dst)
        
    """
    def output_dir(self) -> str:
        if not self.inplace:
            return os.path.join(self.get_finalized_command('build').build_platlib, 'pyomvbb')

        build_py = self.get_finalized_command('build_py')
        package_dir = os.path.abspath(build_py.get_package_dir('pyomvbb'))
        return package_dir
    """


def find_all_files(path: str) -> Iterator[str]:
    for dirpath, _, filenames in os.walk(path):
        for filename in filenames:
            yield os.path.join(dirpath, filename)

content = ""
try:
    descriptor = open("readme.md", "r")
    content = descriptor.read()
    descriptor.close()
except:
    content = ""
requirements = []
setup(name='pyomvbb',
      version='1.0',
      maintainer="Pierce L. Brooks",
      maintainer_email="piercebrks@gmail.com",
      author="Pierce L. Brooks",
      author_email="piercebrks@gmail.com",
      url="https://github.com/PierceLBrooks/pyomvbb",
      ext_modules=[StubExtension("omvbb.__init__")],
      description="pyomvbb",
      long_description=content,
      long_description_content_type="text/markdown",
      classifiers=["Environment :: Console",
                   "Intended Audience :: Developers",
                   "Topic :: Software Development"],
      packages=["pyomvbb"],
      install_requires=requirements,
      zip_safe=False,
      distclass=ShortCircuitDistribution,
      cmdclass={'build_ext': build_ext, "install_data": InstallDataToLibDir},
      data_files=[("omvbb", list(find_all_files("install_data")))]
)

