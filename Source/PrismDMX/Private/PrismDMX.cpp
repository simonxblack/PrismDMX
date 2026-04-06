// ========================================================================================
// PRISM DMX: Pixel Remote Interface Service Modulator
// Author: Simon Schwarz (simonxblack@outlook.com) | April 2026, Amberg, Germany
// AI-Assisted: Coded with Antigravity / Gemini 3.1 Pro (High)
// Bachelor Thesis: "Workflow-Entwicklung und -Testung für DMX Pixel Mapping in
// Virtual Production mit Unreal Engine" East Bavarian Technical University
// Amberg-Weiden (Media production and media technology)
// ========================================================================================
// Description: Universal Modulator for CIE1931 fixtures. Enables external DMX
// console override (e.g. Blackout) for DMX Pixel Mapping in Unreal Engine via
// custom Shifter Address.
// ========================================================================================

#include "PrismDMX.h"
#include "DMXProtocolTypes.h"
#include "DMXSubsystem.h"
#include "Engine/Engine.h"
#include "IO/DMXInputPort.h"
#include "IO/DMXInputPortReference.h"
#include "IO/DMXPortManager.h"
#include "Library/DMXEntityFixturePatch.h"
#include "Library/DMXEntityFixtureType.h"
#include "Modules/ModuleManager.h"

UPrismDMX::UPrismDMX() {
  IntensityAttribute.Name = "Dimmer";
  CxAttribute.Name = "CIE_X";
  CyAttribute.Name = "CIE_Y";
  CCTAttribute.Name = "CTC";
  TintAttribute.Name = "Tint";
  ColorCrossFadeAttribute.Name = "Color FX";
}

bool UPrismDMX::FetchDMXBuffers(UDMXEntityFixturePatch *FixturePatch,
                                float &OutShifterValue,
                                FDMXSignalSharedPtr &OutFixtureSignal) {
  // ==========================================
  // 1. CACHE & EVALUATE SHIFTER ADDRESS
  // ==========================================
  // String parsing is cached to avoid expensive operations every frame.
  // We only parse the universe and channel if the address string changes.
  if (PixelMappingShifterAddress != CachedAddressString) {
    FString UnivStr, ChanStr;
    if (PixelMappingShifterAddress.Split(TEXT("."), &UnivStr, &ChanStr)) {
      CachedShifterUniverse = FCString::Atoi(*UnivStr);
      CachedShifterChannel = FCString::Atoi(*ChanStr);
    } else {
      CachedShifterUniverse = 1; // Fallback universe
      CachedShifterChannel = FCString::Atoi(*PixelMappingShifterAddress);
    }
    CachedAddressString = PixelMappingShifterAddress;
  }

  int32 FixtureUniverseID = FixturePatch->GetUniverseID();
  FDMXSignalSharedPtr ShifterSignal;
  bool bFoundShifter = false;
  bool bFoundFixture = false;

  for (const FDMXInputPortSharedRef &InputPort :
       FDMXPortManager::Get().GetInputPorts()) {
    if (!bFoundShifter && InputPort->GameThreadGetDMXSignal(
                              CachedShifterUniverse, ShifterSignal)) {
      bFoundShifter = true;
    }
    if (!bFoundFixture && InputPort->GameThreadGetDMXSignal(FixtureUniverseID,
                                                            OutFixtureSignal)) {
      bFoundFixture = true;
    }
    if (bFoundShifter && bFoundFixture) {
      break;
    }
  }

  OutShifterValue = 0.0f;
  int32 ShifterIndex = CachedShifterChannel - 1;

  // Safely extract the shifter value, preventing out-of-bounds memory access
  if (ShifterSignal.IsValid() &&
      ShifterSignal->ChannelData.IsValidIndex(ShifterIndex)) {
    OutShifterValue = ShifterSignal->ChannelData[ShifterIndex] / 255.0f;
  } else if (bEnableDebug && GEngine) {
    FString WarnStr =
        FString::Printf(TEXT("========== [ %s ] ==========\n[WARNING] No DMX "
                             "data received on Shifter Address '%s'!"),
                        *FixturePatch->GetName(), *PixelMappingShifterAddress);
    GEngine->AddOnScreenDebugMessage(
        (uint64)GetTypeHash(FixturePatch->GetFName()) + 11, 0.0f,
        FColor::Yellow, WarnStr);
  }

  // ==========================================
  // 2. VALIDATE FIXTURE DMX DATA
  // ==========================================
  if (bEnableDebug &&
      (!OutFixtureSignal.IsValid() ||
       OutFixtureSignal->ChannelData.Num() == 0) &&
      GEngine) {
    FString WarnStr =
        FString::Printf(TEXT("========== [ %s ] ==========\n[WARNING] Fixture "
                             "Universe Buffer is empty!"),
                        *FixturePatch->GetName());
    GEngine->AddOnScreenDebugMessage(
        (uint64)GetTypeHash(FixturePatch->GetFName()) + 12, 0.0f,
        FColor::Yellow, WarnStr);
  }

  return true;
}

void UPrismDMX::Modulate_Implementation(
    UDMXEntityFixturePatch *FixturePatch,
    const TMap<FDMXAttributeName, float> &InNormalizedAttributeValues,
    TMap<FDMXAttributeName, float> &OutNormalizedAttributeValues) {

  // ==========================================
  // 1. INITIALIZATION & SAFETY CHECKS
  // ==========================================
  OutNormalizedAttributeValues = InNormalizedAttributeValues;
  if (!FixturePatch) {
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(
          1001, 0.0f, FColor::Red,
          TEXT("PrismDMX CRITICAL: FixturePatch is Invalid!"));
    return;
  }

  const FDMXFixtureMode *Mode = FixturePatch->GetActiveMode();
  if (!Mode) {
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(
          (uint64)GetTypeHash(FixturePatch->GetFName()) + 1002, 0.0f,
          FColor::Red,
          FString::Printf(
              TEXT("PrismDMX CRITICAL [%s]: Active Mode is Invalid!"),
              *FixturePatch->GetName()));
    return;
  }

  // ==========================================
  // 2. FETCH SHIFTER AND FIXTURE DATA
  // ==========================================
  float ShifterValue = 0.0f;
  FDMXSignalSharedPtr FixtureSignal;
  FetchDMXBuffers(FixturePatch, ShifterValue, FixtureSignal);

  // ==========================================
  // 3. PARSE DMX CONSOLE VALUES
  // ==========================================
  // Values are parsed from the console DMX buffer based on the active fixture
  // mode.
  float ConsoleIntensity = 0.0f, ConsoleCx = 0.0f, ConsoleCy = 0.0f;
  float ConsoleCCT = 0.0f, ConsoleTint = 0.0f, ConsoleColorCrossFade = 0.0f;
  bool bFoundIntensity = false;
  const bool bHasMatrixCellAttributes =
      Mode->FixtureMatrixConfig.CellAttributes.Num() > 0;

  for (const FDMXFixtureFunction &Func : Mode->Functions) {
    if (Func.Attribute == IntensityAttribute) {
      bFoundIntensity = true;
      break;
    }
  }

  // Matrix fixtures can define intensity only as cell attribute.
  if (!bFoundIntensity && bHasMatrixCellAttributes) {
    for (const FDMXFixtureCellAttribute &CellAttr :
         Mode->FixtureMatrixConfig.CellAttributes) {
      if (CellAttr.Attribute == IntensityAttribute) {
        bFoundIntensity = true;
        break;
      }
    }
  }

  if (FixtureSignal.IsValid()) {
    const TArray<uint8> &FixtureUniverseBuffer = FixtureSignal->ChannelData;
    int32 StartAddress = FixturePatch->GetStartingChannel() - 1;

    if (Mode->Functions.Num() > 0) {
      for (const FDMXFixtureFunction &Func : Mode->Functions) {
        int32 AbsoluteAddr = StartAddress + Func.Channel - 1;

        // Bounds check to avoid memory access violations
        if (!FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr))
          continue;

        float FinalVal = 0.0f;
        if (Func.DataType == EDMXFixtureSignalFormat::E8Bit) {
          FinalVal = FixtureUniverseBuffer[AbsoluteAddr] / 255.0f;
        } else if (Func.DataType == EDMXFixtureSignalFormat::E16Bit &&
                   FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr + 1)) {
          uint32 High = FixtureUniverseBuffer[AbsoluteAddr];
          uint32 Low = FixtureUniverseBuffer[AbsoluteAddr + 1];
          FinalVal = ((High << 8) | Low) / 65535.0f;
        }

        // Map retrieved values directly to console variables
        if (Func.Attribute == IntensityAttribute)
          ConsoleIntensity = FinalVal;
        else if (Func.Attribute == CxAttribute)
          ConsoleCx = FinalVal;
        else if (Func.Attribute == CyAttribute)
          ConsoleCy = FinalVal;
        else if (Func.Attribute == CCTAttribute)
          ConsoleCCT = FinalVal;
        else if (Func.Attribute == TintAttribute)
          ConsoleTint = FinalVal;
        else if (Func.Attribute == ColorCrossFadeAttribute)
          ConsoleColorCrossFade = FinalVal;
      }
    } else if (bHasMatrixCellAttributes) {
      // Fallback for matrix-based fixture modes when Modulate_Implementation
      // is used: read first cell attributes.
      const int32 CellStartIndex = Mode->FixtureMatrixConfig.FirstCellChannel - 1;
      int32 LocalAttrOffset = 0;

      for (const FDMXFixtureCellAttribute &CellAttr :
           Mode->FixtureMatrixConfig.CellAttributes) {
        const int32 AbsoluteAddr = StartAddress + CellStartIndex + LocalAttrOffset;

        float FinalVal = 0.0f;
        if (CellAttr.DataType == EDMXFixtureSignalFormat::E8Bit) {
          if (FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr)) {
            FinalVal = FixtureUniverseBuffer[AbsoluteAddr] / 255.0f;
          }
          LocalAttrOffset += 1;
        } else if (CellAttr.DataType == EDMXFixtureSignalFormat::E16Bit) {
          if (FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr + 1)) {
            uint32 High = FixtureUniverseBuffer[AbsoluteAddr];
            uint32 Low = FixtureUniverseBuffer[AbsoluteAddr + 1];
            FinalVal = ((High << 8) | Low) / 65535.0f;
          }
          LocalAttrOffset += 2;
        } else {
          LocalAttrOffset += 1;
        }

        if (CellAttr.Attribute == IntensityAttribute)
          ConsoleIntensity = FinalVal;
        else if (CellAttr.Attribute == CxAttribute)
          ConsoleCx = FinalVal;
        else if (CellAttr.Attribute == CyAttribute)
          ConsoleCy = FinalVal;
        else if (CellAttr.Attribute == CCTAttribute)
          ConsoleCCT = FinalVal;
        else if (CellAttr.Attribute == TintAttribute)
          ConsoleTint = FinalVal;
        else if (CellAttr.Attribute == ColorCrossFadeAttribute)
          ConsoleColorCrossFade = FinalVal;
      }
    }
  }

  // Warning: Notify user if the essential Intensity attribute is unmapped
  if (!bFoundIntensity && IntensityAttribute.IsValid() && GEngine) {
    FString WarnMsg = FString::Printf(
        TEXT("PrismDMX CRITICAL [%s]: Attribute '%s' missing in Fixture Mode!"),
        *FixturePatch->GetName(), *IntensityAttribute.Name.ToString());
    GEngine->AddOnScreenDebugMessage(
        (uint64)GetTypeHash(FixturePatch->GetFName()) + 1003, 0.0f, FColor::Red,
        WarnMsg);
  }

  // ==========================================
  // 4. MODULATION & OUTPUT
  // ==========================================
  const float ShifterThreshold = 10.0f / 255.0f;

  if (ShifterValue > ShifterThreshold) {
    // ----------------------------------------
    // MODE A: PIXEL MAPPING PROPORTIONAL MIX
    // ----------------------------------------
    const float *PixelIntensityPtr =
        InNormalizedAttributeValues.Find(IntensityAttribute);
    float PixelIntensity = PixelIntensityPtr ? *PixelIntensityPtr : 0.0f;

    float NewIntensity =
        (PixelIntensity * IntensityMultiplier * ConsoleIntensity) - 1.0f +
        ShifterValue;

    if (bEnableDebug && GEngine) {
      FString DebugStr = FString::Printf(
          TEXT("========== [ %s ] ==========\nMode: Pixel Mapping "
               "Active\nIntensity: %.2f  (Pix: %.2f * M: %.2f * Cons: %.2f - 1 "
               "+ Shft: %.2f)"),
          *FixturePatch->GetName(), NewIntensity, PixelIntensity,
          IntensityMultiplier, ConsoleIntensity, ShifterValue);

      FColor DebugColor = FColor::Cyan;
      if (NewIntensity > 1.0f) {
        DebugStr +=
            TEXT("\n[!] Max Intensity Exceeded! Please reduce at console.");
        DebugColor = FColor::Red;
      } else if (NewIntensity < 0.0f) {
        DebugStr +=
            TEXT("\n[!] Min Intensity Clipped! Please increase at console.");
        DebugColor = FColor::Blue;
      }

      GEngine->AddOnScreenDebugMessage(
          (uint64)GetTypeHash(FixturePatch->GetFName()), 0.0f, DebugColor,
          DebugStr);
    }

    OutNormalizedAttributeValues.Add(IntensityAttribute,
                                     FMath::Clamp(NewIntensity, 0.0f, 1.0f));
  } else {
    // ----------------------------------------
    // MODE B: CONSOLE OVERRIDE (BLACKOUT/DIRECT)
    // ----------------------------------------
    if (bEnableDebug && GEngine) {
      FString DebugStr = FString::Printf(
          TEXT("========== [ %s ] ==========\nMode: Console Override "
               "Active\nIntensity: %.2f | Cx: %.2f | Cy: %.2f"),
          *FixturePatch->GetName(), ConsoleIntensity, ConsoleCx, ConsoleCy);
      GEngine->AddOnScreenDebugMessage(
          (uint64)GetTypeHash(FixturePatch->GetFName()), 0.0f, FColor::Orange,
          DebugStr);
    }
    OutNormalizedAttributeValues.Add(CxAttribute, ConsoleCx);
    OutNormalizedAttributeValues.Add(CyAttribute, ConsoleCy);
    OutNormalizedAttributeValues.Add(IntensityAttribute, ConsoleIntensity);
  }

  OutNormalizedAttributeValues.Add(CCTAttribute, ConsoleCCT);
  OutNormalizedAttributeValues.Add(TintAttribute, ConsoleTint);
  OutNormalizedAttributeValues.Add(ColorCrossFadeAttribute,
                                   ConsoleColorCrossFade);
}

void UPrismDMX::ModulateMatrix_Implementation(
    UDMXEntityFixturePatch *FixturePatch,
    const TArray<FDMXNormalizedAttributeValueMap>
        &InNormalizedMatrixAttributeValues,
    TArray<FDMXNormalizedAttributeValueMap>
        &OutNormalizedMatrixAttributeValues) {

  // ==========================================
  // 1. INITIALIZATION & SAFETY CHECKS
  // ==========================================
  OutNormalizedMatrixAttributeValues = InNormalizedMatrixAttributeValues;
  if (!FixturePatch) {
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(
          2001, 0.0f, FColor::Red,
          TEXT("PrismDMX Matrix CRITICAL: FixturePatch is Invalid!"));
    return;
  }

  const FDMXFixtureMode *Mode = FixturePatch->GetActiveMode();
  if (!Mode) {
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(
          (uint64)GetTypeHash(FixturePatch->GetFName()) + 2002, 0.0f,
          FColor::Red,
          FString::Printf(
              TEXT("PrismDMX Matrix CRITICAL [%s]: Active Mode is Invalid!"),
              *FixturePatch->GetName()));
    return;
  }

  // ==========================================
  // 2. FETCH SHIFTER AND FIXTURE DATA
  // ==========================================
  float ShifterValue = 0.0f;
  FDMXSignalSharedPtr FixtureSignal;
  FetchDMXBuffers(FixturePatch, ShifterValue, FixtureSignal);

  // ==========================================
  // 3. CACHE MATRIX CONSTANTS
  // ==========================================
  // Pre-calculate invariant metrics to avoid redundant iterations per cell
  int32 StartAddress = FixturePatch->GetStartingChannel() - 1;
  int32 CellStartIndex = Mode->FixtureMatrixConfig.FirstCellChannel - 1;
  int32 NumCells = OutNormalizedMatrixAttributeValues.Num();

  int32 ChannelsPerCell = 0;
  bool bFoundIntensity = false;

  for (const FDMXFixtureCellAttribute &CellAttr :
       Mode->FixtureMatrixConfig.CellAttributes) {
    ChannelsPerCell +=
        (CellAttr.DataType == EDMXFixtureSignalFormat::E16Bit) ? 2 : 1;

    if (CellAttr.Attribute == IntensityAttribute) {
      bFoundIntensity = true;
    }
  }

  // ==========================================
  // 4. PROCESS MATRIX CELLS
  // ==========================================
  for (int32 CellIndex = 0; CellIndex < NumCells; CellIndex++) {
    FDMXNormalizedAttributeValueMap &CellMap =
        OutNormalizedMatrixAttributeValues[CellIndex];
    int32 CurrentCellOffset = CellStartIndex + (CellIndex * ChannelsPerCell);

    float ConsoleIntensity = 0.0f, ConsoleCx = 0.0f, ConsoleCy = 0.0f;
    float ConsoleCCT = 0.0f, ConsoleTint = 0.0f, ConsoleColorCrossFade = 0.0f;

    if (FixtureSignal.IsValid()) {
      const TArray<uint8> &FixtureUniverseBuffer = FixtureSignal->ChannelData;
      // Extract all mapped attributes for the current cell sequentially
      int32 LocalAttrOffset = 0;
      for (const FDMXFixtureCellAttribute &CellAttr :
           Mode->FixtureMatrixConfig.CellAttributes) {
        int32 AbsoluteAddr = StartAddress + CurrentCellOffset + LocalAttrOffset;

        if (FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr)) {
          float FinalVal = 0.0f;
          if (CellAttr.DataType == EDMXFixtureSignalFormat::E8Bit) {
            FinalVal = FixtureUniverseBuffer[AbsoluteAddr] / 255.0f;
            LocalAttrOffset += 1;
          } else if (CellAttr.DataType == EDMXFixtureSignalFormat::E16Bit &&
                     FixtureUniverseBuffer.IsValidIndex(AbsoluteAddr + 1)) {
            uint32 High = FixtureUniverseBuffer[AbsoluteAddr];
            uint32 Low = FixtureUniverseBuffer[AbsoluteAddr + 1];
            FinalVal = ((High << 8) | Low) / 65535.0f;
            LocalAttrOffset += 2;
          } else {
            LocalAttrOffset +=
                (CellAttr.DataType == EDMXFixtureSignalFormat::E16Bit)
                    ? 2
                    : 1; // Fallback safeguard
          }

          if (CellAttr.Attribute == IntensityAttribute)
            ConsoleIntensity = FinalVal;
          else if (CellAttr.Attribute == CxAttribute)
            ConsoleCx = FinalVal;
          else if (CellAttr.Attribute == CyAttribute)
            ConsoleCy = FinalVal;
          else if (CellAttr.Attribute == CCTAttribute)
            ConsoleCCT = FinalVal;
          else if (CellAttr.Attribute == TintAttribute)
            ConsoleTint = FinalVal;
          else if (CellAttr.Attribute == ColorCrossFadeAttribute)
            ConsoleColorCrossFade = FinalVal;
        } else {
          // Increment offset safely to prevent misalignment of subsequent
          // attributes
          LocalAttrOffset +=
              (CellAttr.DataType == EDMXFixtureSignalFormat::E16Bit) ? 2 : 1;

          // Warning: Out-of-bounds access usually indicates a patch/mode
          // configuration error
          if (GEngine && CellIndex == 0) {
            GEngine->AddOnScreenDebugMessage(
                (uint64)GetTypeHash(FixturePatch->GetFName()) + 2003, 0.0f,
                FColor::Red,
                FString::Printf(TEXT("PrismDMX Matrix CRITICAL [%s]: Cell "
                                     "Address exceeds Universe limit!"),
                                *FixturePatch->GetName()));
          }
        }
      }
    }

    // Warning: Notify user if the essential Intensity attribute is missing in
    // the cell definition
    if (!bFoundIntensity && IntensityAttribute.IsValid() && GEngine &&
        CellIndex == 0) {
      FString WarnMsg = FString::Printf(
          TEXT("PrismDMX Matrix CRITICAL [%s]: Attribute '%s' missing in "
               "Cells!"),
          *FixturePatch->GetName(), *IntensityAttribute.Name.ToString());
      GEngine->AddOnScreenDebugMessage(
          (uint64)GetTypeHash(FixturePatch->GetFName()) + 2004, 0.0f,
          FColor::Red, WarnMsg);
    }

    // ==========================================
    // 5. CELL MODULATION & OUTPUT
    // ==========================================
    const float ShifterThreshold = 10.0f / 255.0f;

    if (ShifterValue > ShifterThreshold) {
      const float *PixelIntensityPtr =
          InNormalizedMatrixAttributeValues[CellIndex].Map.Find(
              IntensityAttribute);
      float PixelIntensity = PixelIntensityPtr ? *PixelIntensityPtr : 0.0f;

      float NewIntensity =
          (PixelIntensity * IntensityMultiplier * ConsoleIntensity) - 1.0f +
          ShifterValue;

      if (bEnableDebug && GEngine && CellIndex == 0) {
        FString DebugStr = FString::Printf(
            TEXT("========== [ %s ] ==========\nMode: Matrix Pixel Mapping "
                 "Active\nCell 0 Intensity: %.2f  (Pix: %.2f * M: %.2f * Cons: "
                 "%.2f - 1 + Shft: %.2f)"),
            *FixturePatch->GetName(), NewIntensity, PixelIntensity,
            IntensityMultiplier, ConsoleIntensity, ShifterValue);

        FColor DebugColor = FColor::Cyan;
        if (NewIntensity > 1.0f) {
          DebugStr +=
              TEXT("\n[!] Matrix Max Intensity Exceeded! Reduce at console.");
          DebugColor = FColor::Red;
        } else if (NewIntensity < 0.0f) {
          DebugStr +=
              TEXT("\n[!] Matrix Min Intensity Clipped! Increase at console.");
          DebugColor = FColor::Blue;
        }

        GEngine->AddOnScreenDebugMessage(
            (uint64)GetTypeHash(FixturePatch->GetFName()), 0.0f, DebugColor,
            DebugStr);
      }

      CellMap.Map.Add(IntensityAttribute,
                      FMath::Clamp(NewIntensity, 0.0f, 1.0f));
    } else {
      if (bEnableDebug && GEngine && CellIndex == 0) {
        FString DebugStr = FString::Printf(
            TEXT("========== [ %s ] ==========\nMode: Matrix Console Override "
                 "Active\nCell 0 Intensity: %.2f | Cx: %.2f | Cy: %.2f"),
            *FixturePatch->GetName(), ConsoleIntensity, ConsoleCx, ConsoleCy);
        GEngine->AddOnScreenDebugMessage(
            (uint64)GetTypeHash(FixturePatch->GetFName()), 0.0f, FColor::Orange,
            DebugStr);
      }
      CellMap.Map.Add(CxAttribute, ConsoleCx);
      CellMap.Map.Add(CyAttribute, ConsoleCy);
      CellMap.Map.Add(IntensityAttribute, ConsoleIntensity);
    }

    CellMap.Map.Add(CCTAttribute, ConsoleCCT);
    CellMap.Map.Add(TintAttribute, ConsoleTint);
    CellMap.Map.Add(ColorCrossFadeAttribute, ConsoleColorCrossFade);
  }
}

IMPLEMENT_MODULE(FDefaultModuleImpl, PrismDMX)
