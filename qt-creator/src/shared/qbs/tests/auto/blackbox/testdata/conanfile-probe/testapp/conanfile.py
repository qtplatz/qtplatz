from conans import ConanFile

class TestApp(ConanFile):
    name = "testapp"
    description = "Our project package, to be inspected by the Qbs ConanfileProbe"
    license = "none"
    version = "6.6.6"

    settings = "os"
    options = {"opt": [True, False], "forceFailure": [True, False]}
    default_options = {"opt": False, "forceFailure": False}

    requires = "testlib/1.2.3@qbs/testing"

    def configure(self):
        assert(not self.options.forceFailure)
        self.options["testlib"].opt = self.options.opt

    def source(self):
        pass

    def build(self):
        pass

    def package(self):
        pass
