import "../../qbs-module-providers-helpers.js" as Helpers

ModuleProvider {
    relativeSearchPaths: {
        Helpers.writeModule(outputBaseDir, "qbsmetatestmodule", "from_provider_a");
        return "";
    }
}
