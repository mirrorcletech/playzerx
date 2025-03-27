# PlayzerX

This release includes binaries for both Windows and Ubuntu platforms.

## Download instructions

Download the archive corresponding to your platform from the binaries listed below.

## Windows installation

1. Download the `PlayzerX-[VERSION]-Windows-x86_64.zip` file.
2. Extract the ZIP archive to a location of your choice.
3. Navigate to the extracted folder and run the PlayzerX executable.

## Linux (Ubuntu) installation

1. Download the `PlayzerX-[VERSION]-Ubuntu-22.04-x86_64.tar.gz` file.
2. Open a terminal in the download location and extract:
   ```
   tar -xzvf PlayzerX-[VERSION]-Ubuntu-22.04-x86_64.tar.gz
   ```
3. Navigate to the extracted directory:
   ```
   cd PlayzerX-[VERSION]
   ```
4. Make the application executable (if needed):
   ```
   chmod +x PlayzerX
   ```
5. Run the application:
   ```
   ./PlayzerX
   ```
6. For USB control mode, ensure your device is connected via micro-USB port.
7. If connecting with Analog Input mode, ensure proper connections to your external controller.

## Interface Modes

PlayzerX provides two interface modes:

1. **Analog Input**: Control laser beam direction (X, Y) and intensity using analog signals. Requires external hardware (NIDAQ, FPGA/MCU, etc.).

2. **USB Input**: Connect directly to your PC via micro-USB for both power and serial communication. Use the Binary Mode protocol for efficient command encoding.

## Documentation

For complete documentation including specifications, electrical requirements, and API details, please refer to:
- [Mirrorcle Playzer X-Series User Guide](https://mirrorcletech.com/pdf/PX/Mirrorcle_Playzer_X-Series_-_User_Guide.pdf)
- [PlayzerX Documentation](https://mirrorcletech.github.io/playzerx/)

## Support

For technical support, please contact support@mirrorcletech.com or visit [Mirrorcle Technologies](https://www.mirrorcletech.com/wp/contact/).
