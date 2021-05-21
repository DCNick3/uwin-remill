from conans import ConanFile, CMake, tools, RunEnvironment
import os
from pathlib import Path

class TestPackageConan(ConanFile):
    settings = "os", "build_type", "arch", "compiler"    

    def test(self):
        self.output.info("cwd = %s" % os.getcwd())
        self.output.info("source_folder = %s" % self.source_folder)

        src = Path(self.source_folder)

        self.run(['uwin-lift-hlp', src / 'test.exe', 'test.o'])
        self.run(['file', 'test.o'])
