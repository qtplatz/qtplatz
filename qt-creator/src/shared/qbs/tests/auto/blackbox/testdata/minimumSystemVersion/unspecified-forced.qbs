import qbs.Utilities

// no minimum versions are specified, and explicitly set to undefined in
// case the profile has set it
import qbs.Host

CppApplication {
    condition: {
        var result = qbs.targetPlatform === Host.platform();
        if (!result)
            console.info("targetPlatform differs from hostPlatform");
        return result;
    }
    files: [qbs.targetOS.includes("darwin") ? "main.mm" : "main.cpp"]
    consoleApplication: true
    cpp.minimumWindowsVersion: undefined
    cpp.minimumMacosVersion: undefined
    cpp.minimumIosVersion: undefined
    cpp.minimumAndroidVersion: undefined

    Properties {
        condition: qbs.targetOS.includes("windows")
        cpp.defines: ["TOOLCHAIN_INSTALL_PATH=" + Utilities.cStringQuote(cpp.toolchainInstallPath)]
    }

    Properties {
        condition: qbs.targetOS.includes("darwin")
        cpp.frameworks: "Foundation"
    }
}
