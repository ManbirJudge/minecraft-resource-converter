# Minecraft Reource Converter
This is a simple application that can convert your Minecraft Java resource packs to be used on Minecraft Bedrock i.e. Minecraft PE, Minecraft for XBoxe, Minecraft for PS, Mincraft for Windows 10 and others.


## TODOS
### Symbols
- In progress, coming soon - 🏃
- In progress - ⏳
- Not in progress - 🚧
- No plans yet - 🤔
- Never to be added - ❌

### Features
- Progress bar to show conversion progress. ⏳
- Better building process. ⏳
- Better distribution process. 🚧
- Better documentation. 🚧
- Installer. 🤔

### Resources
- Homepage title 🚧
- Fonts 🚧
- Paintings 🚧
- Sounds 🚧
- Sheep textures 🚧
- Entity textures ⏳
- Custom models (optifine equivalent) ❌
    
## Building from source
`Tested using Qt MinGW Kit on Windows 10 and 11.`
1. Prequesties - OpenCV DLLs, libzip and zlib.
2. Open the project using Qt Creator.
3. In the project `.pro` file, change the paths to zlib and libzip include and library directories.
4. Build it using the Qt Creator.

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
1. Clone/download the repositry.
2. Grab the `bin/Windows x64.rar` and extract it to location (generally in `%USER%` folder)
3. Run the `minecraft-resource-converter.exe`
4. Additionally, creator a shortcut to the executable on your Desktop, Taskbar or Start Menu.

## Contributors
- Me
