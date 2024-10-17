from conan import ConanFile
from conan.tools.cmake import cmake_layout


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def configure(self):
        self.settings.compiler.cppstd = "gnu20"

    def requirements(self):
        self.requires("folly/2024.08.12.00")
        self.requires("gtest/1.15.0")
        self.requires("benchmark/1.9.0")

    def layout(self):
        cmake_layout(self)
