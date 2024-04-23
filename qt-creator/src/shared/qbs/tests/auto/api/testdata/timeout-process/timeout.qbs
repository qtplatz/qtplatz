import qbs.Host

Project {
    CppApplication {
        type: "application"
        consoleApplication: true // suppress bundle generation
        files: "main.cpp"
        name: "infinite-loop"
        cpp.cxxLanguageVersion: "c++11"
        cpp.minimumOsxVersion: "10.8" // For <chrono>
        Properties {
            condition: qbs.toolchain.includes("gcc")
            cpp.driverFlags: "-pthread"
        }
    }

    Product {
        condition: {
            var result = qbs.targetPlatform === Host.platform();
            if (!result)
                console.info("targetPlatform differs from hostPlatform");
            return result;
        }
        type: "product-under-test"
        name: "caller"
        Depends { name: "infinite-loop" }
        Depends {
            name: "cpp" // Make sure build environment is set up properly.
            condition: Host.os().includes("windows") && qbs.toolchain.includes("gcc")
        }
        Rule {
            inputsFromDependencies: "application"
            outputFileTags: "product-under-test"
            prepare: {
                var cmd = new Command(inputs["application"][0].filePath);
                cmd.description = "calling application that runs forever";
                cmd.timeout = 3;
                return cmd;
            }
        }
    }
}
