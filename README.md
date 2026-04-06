# PrismDMX Unreal Engine Plugin

**PRISM DMX: Pixel Remote Interface Service Modulator**
Author: Simon Schwarz
Bachelor Thesis: "Workflow-Entwicklung und -Testung für DMX Pixel Mapping in Virtual Production mit Unreal Engine"
East Bavarian Technical University Amberg-Weiden

## Overview
PrismDMX is a universal Modulator for CIE1931 fixtures in Unreal Engine. It enables external DMX console override (e.g., Blackout) for DMX Pixel Mapping in Unreal Engine via a custom Shifter Address. It handles regular and matrix features directly.

## Features
- **External Modulator / Override:** Switch dynamically between UE Pixel Mapping output and external Lighting Console controls, like Blackout or Dimmer control.
- **Support for Matrix Fixtures:** Allows handling of matrix/multicell fixture types.
- **Cross-Platform & Version Independent:** Designed with C++ to easily work on Windows, Linux, and macOS platforms. Keep it updated by simply recompiling it for new major UE versions.
- **nDisplay Support:** Perfect for Virtual Production LED Volumes. The stateless design enables seamless multi-node DMX processing across nDisplay clusters matching UE's standard pixel mapping mechanism.

## Installation Instructions

1. **Download the Plugin:**
   Download or clone this repository directly. Keep only the necessary files to avoid bloat.

2. **Add to your Unreal Engine Project:**
   In the root directory of your Unreal Engine project (where your `.uproject` file is located), find the `Plugins/` folder. If it does not exist, create it.
   Place the downloaded `PrismDMX` folder into this `Plugins/` folder.

3. **Generate Visual Studio Project Files:**
   Right-click your `.uproject` file and select **"Generate Visual Studio project files"** (or use the equivalent process if you are on MacOS or using JetBrains Rider).

4. **Build the Project:**
   Open the generated project solution in your IDE (e.g., Visual Studio), and compile the project (Build or Run). The UE Editor will start up once compilation is successful. 

5. **Enable the Plugin:**
   Inside the Unreal Editor, go to `Edit -> Plugins`.
   Search for `PrismDMX` under the 'Project' section or the installed plugins list. Make sure it's enabled. Restart the editor if required.

## Documentation & Usage

Once the plugin is enabled, you can add `PrismDMX` as a Modulator class.
Configuration settings inside the editor allow setting:
- **Shifter Address (Universe.Channel)**
- **Intensity Multiplier** 
- **Debug modes to verify incoming DMX values**

Have fun with seamless DMX Pixel mapping and Console modulation using Unreal Engine!
