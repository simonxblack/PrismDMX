# 💎 PrismDMX: Unreal Engine Plugin

**PRISM DMX: Pixel Remote Interface Service Modulator**

---

### ℹ️ Project Information
* **Developer:** Simon Schwarz
* **University:** East Bavarian Technical University Amberg-Weiden (OTH AW)
* **Bachelor Thesis:** *"Workflow Development and Testing for DMX Pixel Mapping in Virtual Production with Unreal Engine"*
* **Degree:** Media Production and Media Technology (B.Eng.)
* **Implementation:** Conceptual logic developed by the author; Large Language Models (AI) were utilized as assistive tools for technical C++ implementation and syntax optimization.

---

## 📖 Overview
**PrismDMX** is a universal modulator designed for **CIE1931** fixtures within Unreal Engine. It enables an external DMX console override (e.g., Blackout) for DMX Pixel Mapping via a custom **Shifter Address**. The plugin handles both standard and matrix/multicell fixture features directly.

## ✨ Features
* **External Modulator / Override:** Dynamically switch between native Unreal Engine Pixel Mapping output and external lighting console control.
* **Support for Matrix Fixtures:** Built-in handling for matrix and multicell fixture types.
* **Cross-Platform & Version Independent:** Designed with C++ for Windows, Linux, and macOS. Simply recompile for new major Unreal Engine versions to keep it up to date.

---

## 📥 Installation

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

## ⚙️ Setup

To integrate **PrismDMX** into your workflow, follow these steps for both your lighting console and Unreal Engine:

### 🎛️ 1. Lighting Console Configuration
* **Create Shifter:** Patch one or more **Shifters** (configured as a standard **Dimmer** fixture) into any desired DMX universe.
* **Fixture Patching:** All Pixel Mapping fixtures must be assigned to a separate DMX universe. Multiple Pixel Mapping fixtures can share the same universe, but no non-Pixel Mapping fixtures can be used within it.

### 💻 2. Unreal Engine Configuration
* **Patch Alignment:** The fixture configuration (Patch, Universe, and Mode) in your Unreal Engine setup must match the setup of your lighting console exactly.
* **Color Space Mode:** Ensure that the **Color Space Output Mode** in your DMX Pixel Mapping Plugin settings is set to **CIE 1931 xyY**.
* **Fixture Setup:** For each fixture intended for Pixel Mapping, perform the following settings:
    * **Add Modulator:** Within your DMX Pixel Mapping Plugin settings, add an **Output Modulator** and select `PrismDMX` from the dropdown list.
    * **Configure Shifter:** Set the **Shifter Address** (Universe.Channel) in the Modulator settings to match the Shifter patched in your lighting console.
    * **Attribute Matching:** Ensure that the **Attribute Names** match those used in your **DMX Library** exactly.
    * **Debugging:** If necessary, enable **Debug Mode** within the Modulator settings.

> [!TIP]
> **Shifter Flexibility:** A single Shifter can control a group of fixtures simultaneously by assigning the same DMX address to multiple fixtures within Unreal Engine.

---

## 🚀 Usage

Once the setup is complete, the behavior of the modulator is dynamically controlled via the **Shifter** value:

### 🔄 Operating Modes
The behavior of the modulator is dynamically controlled via the **Shifter** value:

| Mode | Shifter Value | Control Logic |
| :--- | :--- | :--- |
| **Standard Mode** | `0` | Controlled exclusively by the lighting console (Intensity, Color Coordinates, CCT, Tint, Color Cross Fade). |
| **Pixel Mapping Mode** | `0.1 – 1.0` | Console color coordinates are replaced by UE Pixel Mapping data. CCT, Tint, and Cross Fade remain console-controllable. |

### 🧮 Intensity Recalculation
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
