# gst-freeze
A Gstreamer plugin to freeze the displayed image. Used in camera_gui application

# Version 1.0

# About

This plugin allows to freeze the video stream according a parameter in the gstreamer pipeline or by a manual user entry

# Dependencies

The following libraries are required for this plugin.
- v4l-utils
- libv4l-dev
- libgstreamer1.0-dev
- libgstreamer-plugins-base1.0-dev
- meson
#### Debian based system (Jetson): 

```
sudo apt install v4l-utils libv4l-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev meson
```
##### Note : if you are using a Yocto distribution, look at the github to find a .bbappend file which provides all packages to your distribution 


# Compilation

## Ubuntu (Jetson)
First you must make sure that your device's clock is correctly setup.
Otherwise the compilation will fail.

### Using Meson 

In the **gst-freeze** folder do:

```
meson build
```
```
ninja -C build
```
```
sudo ninja -C build install
```


## Yocto (IMX)
First you must make sure that your device's clock is correctly setup.
Otherwise the compilation will fail.

### Using Meson 

In the **gst-freeze** folder do:

```
meson build
```
```
ninja -C build install
```

# Installation test

To test if the plugin has been correctly install, do:
```
export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0/
gst-inspect-1.0 freeze
```

If the plugin failed to install the following message will be displayed: "No such element or plugin 'freeze'"

# Uninstall
'''
sudo rm /usr/local/lib/gstreamer-1.0/libgstfreeze.*
'''
# Usage

By default the plugin is installed in /usr/local/lib/gstreamer-1.0. 
It is then required to tell gstreamer where to find it with the command:
```
export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0/
```
The plugin can be used in any gstreamer pipeline by adding '''freeze''', the name of the plugin.

# Pipeline examples:
With fake image pipeline:
```
gst-launch-1.0 videotestsrc ! freeze ! videoconvert ! ximagesink
```

With simple video stream:
```
gst-launch-1.0 v4l2src ! freeze ! queue ! videoconvert ! queue ! xvimagesink sync=false
```

By default, the plugin listen for user inputs in the terminal. You can freeze and unfreeze the video stream by entering '''f''' in the terminal.

# Plugin parameters

- freeze:
    - Type: boolean
    - Default value: false
    - Description: Freeze the stream without blocking the pipeline

- listen:
    - Type: boolean
    - Default value: true
    - Description: Listen for user inputs in the terminal
