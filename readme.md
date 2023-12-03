# Minecraft Reource Converter
This is a simple application that can convert your Minecraft Java resource packs to be used on Minecraft Bedrock i.e. Minecraft PE, Minecraft for XBoxe, Minecraft for PS, Mincraft for Windows 10 and others.

## Symbols
- In progress, coming soon - 🏃
- In progress, will take some time - ⏳
- Plans to add, not in progress - 🚧
- No plans to add for now - 🤔
- No plans to add forever - ❌

## TODOS
- Carried textures  ⏳
- All entity textures ⏳
- Fonts ⏳
- Homepage title 🏃
- Paintings ⏳
- Sounds 🚧
- Sheep textures 🚧
- Custom models (optifine equivalent) ❌
- Progress bar to show conversion progress ⏳
- Individual items -
    - Decorated pot
    - Conduit textures
    - Potion textures
    - Bed item texture
    

## Building from source
- Prequesties - OpenCV DLLs, libzip and zlib.
- Open the project using Qt Creator.
- In the project `.pro` file, change the paths to zlib and libzip include and library directories.
- Build it using the Qt Creator.

## DLLs required for Distribution
### Qt Specific
I do not remember the name but Qt has a built-in tool that copies all the linked Qt DLLs (given a Qt generated .exe file) into the working directory.
### Other
These are included with the source.

- libopencv_core460.dll, libopencv_imgcodecs460.dll, libopencv_imgproc460.dll
- zip.dll
- zlib.dll

## Installation
### Windows
1. Download the repositry in anyway
2. Grab the `bin/Windows x64.rar` and extract it to location (generally in `%USER%` folder)
3. Run the `minecraft-resource-converter.exe`

## Contributors
- Me
