import os

from conans import ConanFile, CMake
from conan.tools.files import copy

class agromaster_recipe(ConanFile):
    settings = "arch", "build_type", "compiler", "os"
    generators = "cmake_find_package"
    
    def requirements(self):
        self.requires("wt/4.10.1")
        self.requires("gtest/1.15.0")
        
    
    def generate(self):
        copy(self, "*",
            os.path.join(self.dependencies["wt"].package_folder, "bin", "wt"),
            self.build_folder)
    
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()