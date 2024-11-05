from conan import ConanFile

class ConanTutorialRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "PremakeDeps"

    def requirements(self): 
        self.requires("glfw/3.4")
        self.requires("imgui/cci.20230105+1.89.2.docking")
        self.requires("spdlog/1.14.1")
        self.requires("volk/1.3.296.0")