## litebrowser (litehtml engine)

**Litebrowser** is simple web browser designed to test the [litehtml HTML rendering engine](https://github.com/tordex/litehtml).

### Building

You can build litebrowser with Visual Studio 2022 and newer. **Note**: this project contains some git submodules:

  * [freeimage](https://github.com/tordex/freeimage) - used to draw images
  * [cairo](https://github.com/tordex/cairo) - 2D graphics library
  * [txdib](https://github.com/tordex/txdib) - wrapper for freeimage
  * [simpledib](https://github.com/tordex/simpledib) - library for memory device context and DIBs
  * [litehtml](https://github.com/tordex/litehtml) - html rendering engine

Please be sure the submodules are fetched, or download them from github and copy into libs subfolder.

### Download binaries

You can download the binary files from the [Releases](https://github.com/litehtml/litebrowser/releases) page.

### Using litebrowser

Before running litebrowser copy the files cairo.dll (from libs\cairo) and freeimage.dll (libs\freeimage) into the same folder where litebrowser.exe is.

Type url in the address bar and press [ENTER]
