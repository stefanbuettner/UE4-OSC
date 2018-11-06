UE4-SSI
=======

SSI plugin for Unreal Engine 4 to send and receive SSI events and streams with blueprints!

SSI is a framework for social signal interpretation (hence SSI) from the University of Augsburg. More information at https://hcm-lab.de/projects/ssi/.

It uses oscpack, a C++ library for packing/unpacking OSC messages, by Ross Bencina. http://www.rossbencina.com/code/oscpack


# Integration

Install Visual Studio 2015 on Windows (Community edition works) or XCode on MaxOSX.

Create a "C++" project, or convert a "Blueprint" project to "C++".

Create a "Plugins" directory in the project root (near the "Content" directory) and put the "SSI" folder of the plugin in it.
(e.g. "/MyProject/Plugins/SSI")

Run the "*.uproject" file: the plugin is compiled automatically.

Alternatively, right-clic the "*.uproject" to generate Visual Studio or XCode project files.


# Usage

## Streams

Streams are logged via the components tick methods. The logging frequency can be set via the component's TickInterval.

### Location
The location p = (x, y, z) is send to SSI as the 3 consecutive float values x, y, and z representing the logger's world location in euclidean coordinates.

### Rotation
The rotation q = (x, y, z, w) is send to SSI as the 4 consecutive float values x, y, z, and w representing the logger's rotation as a quaternion in world space.

### Transform
The unscaled transform of an object is send to SSI as a concatenation of the object's location and rotation (x, y, z, qx, qy, qz, qw).

## Sender

Set the "Send Targets" addresses in the plugin settings. (Edit/Project Settings/Plugins/OSC)

You can also add new targets dynamically, using the "Add Send Osc Target" function.
It is especially useful for "ping"-like behavior.

Send a single message with "Send Osc" or a bundle of messages with "Send Osc Bundle".

The "Send Osc" function takes: the OSC address, the OSC parameters and the "target index" of the destination.

Build the OSC parameters for a message:
 - chain PushFloat/Int/Bool/String/Blob functions, or
 - fill an array of "Osc Data Elem" with the standard array functions and the FromFloat/Int/Bool/String/Blob functions.

The "target index" refers to an index in the "Send Targets" array in the settings,
or to the result of the "Add Send Osc Target" function.

## Inputs (experimental)

List the messages that should be UE4 inputs (like a game controller). A new input is created for each message.

Bind OSC inputs to UE4 action in the usual "Input" project settings.


# General information

## Known issues
The plugin currently only supports the UDP protocol.

## Troubleshooting

The plugin uses the standard UE4 logging system.

Check your logs (Window/Developers Tools/Output Log) before asking for help!

## String limitation

Historically, the plugin handles OSC string using "FName". Unfortunately, the length of
a "FName" is limited to 1024 characters. So this plugin does not handle long strings correctly.

If you need long strings, prefer using blobs.

## Blob

The OSC protocol supports buffers as "blob".

This plugin implements them as "Array<uint8>". It plays nicely with the "Extended Standard Library" plugin by "Low Entry".


# Engine version

The current version of the plugin was developed with the Unreal Engine version 4.19.2.

If the plugin does not work/build for the current version, feel free to create an issue or email me!
