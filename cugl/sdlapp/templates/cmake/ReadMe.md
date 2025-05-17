# CMAKE CONFIGURATION DIRECTORY

When the SDL script configures a CMake build, it only creates the CMakeLists.txt file. It does not finish configuring it or use it to compile the build. That is because executing CMake is extremely not cross-platform. That is why we recommend against sharing your CMake build files in your Code repository.

Because of this distinction, this is technically not a build directory. This is a *configuration* directory. You run `cmake` on this directory to create the build directory. To minimize the chance of corruption, we do not allow this directory to also be the build directory. You will need a separate folder for that.

We have, however, provided you a subfolder to use as your build directory. This subfolder has the name `cmake` (appropriately enough). To build your game, simply type the following commands, starting from this directory:

```
cd cmake
cmake ..
cmake --build .
```

The final result will be stored in the `install` folder, which will package the executable together with the assets.

Note that you do not have to actually build your game in the `cmake` folder.  If you know how to use CMake, you can build it anywhere. However, using the `cmake` directory is advantageous for Flatpak builds.

## Creating a Flatpak Release

Flatpak is the preferred format for third-party games on the Steam Deck. It is a wrapper that safely sandboxes your application on the device. There are normally several steps to setting up a Flatpak application, but we have strived to make it as simple as possible for you.

Obviously, to create a flatpak release, you will need `flatpak` (and `flatpak-builder`) installed in your Linux distribution. These are provided by most package managers. In addition, the first time that you create a flatpak build, you will need to install the dependencies. To do that type

```
flatpak install flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08
```

Next, you need to create an executable in the `cmake/install` subdirectory, following the instructions above. Flatpak is a wrapper around this executable, and not a new executable. Once this executable is built, look at the `flatpak` subdirectory of this folder.  You will see a shell script named `build.sh`.  You can run this script *from this directory* as follows:

```
flatpak/build.sh
```

This will create a new subdirectory of this folder named `install`. This directory will contain the following files:

- **`<appid>.flatpak`**: A flatpak bundle of with the app id of your game
- **`<game>.exe`**: A shortcut for launching the game on the Steam Deck
- **`install.sh`**: A script to install the flatpak bundle
- **`uninstall.sh`**: A script to uninstall the flatpak bundle

You should distribute this folder to anyone who wishes to play your game. Once they save they files on their device, they should run the installation script `install.sh`. This will officially install the Flatpak bundle, creating the save directory for the user. They may be prompted for their password to complete the installation.

Once that installation is complete, they can run the game by running the `.exe` file.  This `.exe` file is what they should assign as the *target* if they wish to add the game to their Steam Library. You may want to include custom artwork with this package so that they can properly configure the Steam Page for the game.
