from conans import ConanFile, CMake, tools, RunEnvironment
from pathlib import Path
import os
import shutil

class RemillConan(ConanFile):
    name = "uwin-remill"
    version = "0.1"
    settings = "os", "build_type", "arch"
    url = "https://gitlab.com/uwin-dev/remill"
    license = "Apache"
    description = """Fork of remill to help translate executable code for uwin"""
    generators = "cmake"
    exports_sources = "*"

    requires = "lifting-bits-cxx-common/0.1.4@uwin/stable", "ghidra/9.2.3@uwin/stable"

    def _configure_cmake(self):
        vcpkg_root = self.deps_cpp_info['lifting-bits-cxx-common'].rootpath

        cmake = CMake(self)
        cmake.definitions['VCPKG_ROOT'] = vcpkg_root
        #cmake.definitions['CMAKE_INSTALL_PREFIX'] = self._install_prefix()
        cmake.configure(source_folder=".", )
        return cmake


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        self.run(['python3', '-m', 'pip', 'install', '--ignore-installed', '--upgrade',
                  '--target', Path(self.package_folder) / "lib/python",
                  Path(self.source_folder) / "bin/uwin-lift-hlp" ])
        for f in (Path(self.package_folder) / "lib/python/bin").iterdir():
            shutil.move(f, Path(self.package_folder) / "bin" / f.name)

    def package_info(self):
        self.env_info.PATH.append(str(Path(self.package_folder) / "bin"))
