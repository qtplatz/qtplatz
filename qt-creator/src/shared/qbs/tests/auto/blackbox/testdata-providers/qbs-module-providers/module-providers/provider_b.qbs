import "../../qbs-module-providers-helpers.js" as Helpers

ModuleProvider {
    relativeSearchPaths: {
        Helpers.writeModule(outputBaseDir, "qbsmetatestmodule", "from_provider_b");
        Helpers.writeModule(outputBaseDir, "qbsothermodule", "from_provider_b");
        return "";
    }
}
