## litebrowser (litehtml engine)

**Litebrowser** is simple web browser designed to test the [litehtml HTML rendering engine](https://github.com/tordex/litehtml).

### Building

You can build litebrowser with Visual Studio 2008 and newer. **Note**: this project contains some git submodules:

  * [freeimage](https://github.com/tordex/freeimage) - used to draw images
  * [cairo](https://github.com/tordex/cairo) - 2D graphics library
  * [txdib](https://github.com/tordex/txdib) - wrapper for freeimage
  * [simpledib](https://github.com/tordex/simpledib) - library for memory device context and DIBs
  * [litehtml](https://github.com/tordex/litehtml) - html rendering engine

Please be sure the submodules are fetched, or download them from github and copy into libs subfolder.

### Download binaries

You can download the binary files from [litehtml.com](http://www.litehtml.com).

### Using litebrowser

Before running litebrowser copy the files cairo.dll (from libs\cairo) and freeimage.dll (libs\freeimage) into the same folder where litebrowser.exe is.

Currently the address bar of the litebrowser is *fake*. Run litebrowser with command line parameter:
```
litebrowser.exe http://www.litehtml.com
```

If you run litebrowser without parameter, the dmoz.org will be opened.
