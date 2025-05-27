# Building netcdf-c 4.9.2 on Windows (without HDF5)

This guide describes how to build a minimal version of netCDF-C v4.9.2 on Windows,
sufficient for reading AIA (AndiNetCDF) files without requiring HDF5, DAP, or broken Windows `m4.exe` tools.

---

## 🧱 Dependencies

### zlib

```powershell
git clone https://github.com/madler/zlib.git
cd zlib
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=C:/opt/zlib ..
cmake --build . --config Release --target INSTALL
```

### curl (optional; needed only for DAP — disable features)

```powershell
cd ~/src
wsl
$ wget https://curl.se/download/curl-8.13.0.tar.bz2
$ tar xvf curl-8.13.0.tar.bz2
$ exit

mkdir .\build-x86_64\curl-8.13.0.build
cd .\build-x86_64\curl-8.13.0.build

cmake -DCMAKE_INSTALL_PREFIX="C:/opt/curl-8.13.0" `
      -DZLIB_ROOT="C:/opt/zlib" `
      -DCURL_BROTLI=OFF `
      -DCURL_ZSTD=OFF `
      -DUSE_NGHTTP2=OFF `
      -DUSE_LIBIDN2=OFF `
      -DCURL_USE_LIBPSL=OFF `
      ..\..\curl-8.13.0
```

---

## ⚠️ M4 Requirement and Workaround

The GnuWin32 `m4.exe` is broken (missing `regex2.dll`).
Use **WSL** to generate the needed `.c` files:

```bash
cd netcdf-c/libsrc
m4 -DERANGE_FILL putget.m4 > putget.c
m4 -DERANGE_FILL attr.m4 > attr.c
m4 -DERANGE_FILL ncx.m4 > ncx.c
```

---

## 📦 Build netcdf-c

```powershell
git clone https://github.com/Unidata/netcdf-c.git
cd netcdf-c
git checkout v4.9.2

# Apply patch
git apply ..\..\qtplatz\scripts\windows\netcdf-c_4.9.2.patch

mkdir ..\..\build-x86_64\netcdf-c.build
cd ..\..\build-x86_64\netcdf-c.build

cmake `
  -DCMAKE_PREFIX_PATH="C:/opt/curl-8.13.0" `
  -DCMAKE_INSTALL_PREFIX="C:/opt/netcdf-c" `
  -DZLIB_ROOT="C:/opt/zlib" `
  -DENABLE_TESTS=OFF `
  -DENABLE_NETCDF_4=OFF `
  -DENABLE_DAP=OFF `
  -DBUILD_UTILITIES=OFF `
  -DBUILD_SHARED_LIBS=OFF `
  ..\..\netcdf-c

cmake `
  -DCMAKE_INSTALL_PREFIX="C:/opt/netcdf-c" `
  -DZLIB_ROOT="C:/opt/zlib" `
  -DENABLE_TESTS=OFF `
  -DENABLE_NETCDF_4=OFF `
  -DENABLE_DAP=OFF `
  -DBUILD_UTILITIES=OFF `
  -DBUILD_SHARED_LIBS=OFF `
  ..\..\netcdf-c

cmake --build . --config Release --target INSTALL
```

---

## 📄 Patch: `netcdf-c_4.9.2.patch`

```diff
diff --git a/libsrc/CMakeLists.txt b/libsrc/CMakeLists.txt
index 7b9b2aad9..f5a79e23f 100644
--- a/libsrc/CMakeLists.txt
+++ b/libsrc/CMakeLists.txt
@@ -5,7 +5,8 @@ SET(libsrc_SOURCES v1hpg.c putget.c attr.c nc3dispatch.c
   nc3internal.c var.c dim.c ncx.c lookup3.c ncio.c)

 # Process these files with m4.
-SET(m4_SOURCES attr ncx putget)
+#SET(m4_SOURCES attr ncx putget)
+LIST(APPEND libsrc_SOURCES attr.c ncx.c putget.c)
 foreach (f ${m4_SOURCES})
   GEN_m4(${f} dest)
   LIST(APPEND libsrc_SOURCES ${dest})
```

---

This configuration has been verified to:
- Avoid HDF5, DAP, and plugin dependencies
- Build netcdf-c cleanly on Windows with MSVC and CMake
- Support classic NetCDF-2 files such as Shimadzu AIA/AndiNetCDF exports

Place this file in:
📁 `scripts/windows/netcdf-c_4.9.2_build.md`
