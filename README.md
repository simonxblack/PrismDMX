# PrismDMX: Unreal Engine Plugin

**PRISM DMX: Pixel Remote Interface Service Modulator**

---

### ℹ️ Project Information
* **Developer:** Simon Schwarz
* **University:** East Bavarian Technical University Amberg-Weiden (OTH AW)
* **Bachelor Thesis:** *"Workflow Development and Testing for DMX Pixel Mapping in Virtual Production with Unreal Engine"*
* **Degree:** Media Production and Media Technology (B.Eng.)
* **Implementation:** Conceptual logic developed by the author; Large Language Models (AI) were utilized as assistive tools for technical C++ implementation and syntax optimization.

---

## Overview
**PrismDMX** is a universal modulator designed for **CIE1931** fixtures within Unreal Engine. It enables an external DMX console override (e.g., Blackout) for DMX Pixel Mapping via a custom **Shifter Address**. The plugin handles both standard and matrix/multicell fixture features directly.

## Features
* **External Modulator / Override:** Dynamically switch between native Unreal Engine Pixel Mapping output and external lighting console control.
* **Support for Matrix Fixtures:** Built-in handling for matrix and multicell fixture types.
* **Cross-Platform & Version Independent:** Designed with C++ for Windows, Linux, and macOS. Simply recompile for new major Unreal Engine versions to keep it up to date.

---

## Installation Instructions

1.  **Download the Plugin:**
    Download or clone this repository directly. Keep only the necessary files to avoid bloat.

2.  **Add to your Unreal Engine Project:**
    In the root directory of your Unreal Engine project (where your `.uproject` file is located), find the `Plugins/` folder. If it does not exist, create it. Place the `PrismDMX` folder into this `Plugins/` folder.

3.  **Generate Visual Studio Project Files:**
    Right-click your `.uproject` file and select **"Generate Visual Studio project files"** (or use the equivalent process for macOS or JetBrains Rider).

4.  **Build the Project:**
    Open the generated solution in your IDE (e.g., Visual Studio) and compile the project (Build or Run). The Unreal Editor will launch upon successful compilation.

5.  **Enable the Plugin:**
    Inside the Unreal Editor, go to `Edit -> Plugins`. Search for `PrismDMX` under the 'Project' section and ensure it is enabled. Restart the editor if required.

---

## Usage

Once enabled, you can add `PrismDMX` as a **Output Modulator** within your DMX Pixel Mapping Plugin settings.

### Operating Modes
The behavior of the modulator is dynamically controlled via the **Shifter** value:

| Mode | Shifter Value | Control Logic |
| :--- | :--- | :--- |
| **Standard Mode** | `0` | Controlled exclusively by the lighting console (Intensity, Color Coordinates, CCT, Tint, Color Cross Fade). |
| **Pixel Mapping Mode** | `0.1 – 1.0` | Console color coordinates are replaced by UE Pixel Mapping data. CCT, Tint, and Cross Fade remain console-controllable. |

### Intensity Recalculation
The **Shifter** acts as a subtractive regulator, while the **Console Intensity** serves as a multiplicative factor.

**Normal Position (1:1 Pixel Mapping Output):**
* **Shifter:** 100%
* **Console Intensity:** 50% (assuming the default Multiplier of 2)

#### Formula:

$$New \ Intensity = (Pixel \ Mapping \ Intensity \cdot Multiplier \cdot Console \ Intensity) - 1 + Shifter \ Value$$

> [!NOTE]
> The Multiplier is set to **2** by default and can be adjusted within the **PrismDMX Settings** in the Unreal Editor.

---
*Developed at East Bavarian Technical University Amberg-Weiden (OTH AW)*
