import qbs.Host

Project {
    condition: {
        var result = qbs.targetPlatform === Host.platform();
        if (!result)
            console.info("targetPlatform differs from hostPlatform");
        return result;
    }
    DynamicLibrary {
        name: "testlib"
        Depends { name: "cpp"}
        files: ["testlib.cpp", "testlib.def"]
    }
    CppApplication {
        name: "testapp"
        Depends { name: "testlib"}
        files: ["testapp.cpp"]
    }
}
