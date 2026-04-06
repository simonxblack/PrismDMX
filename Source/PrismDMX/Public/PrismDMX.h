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

#pragma once

#include "CoreMinimal.h"
#include "DMXProtocolTypes.h"
#include "Modulators/DMXModulator.h"
#include "PrismDMX.generated.h"

/**
 * Custom DMX Modulator with Pixel mapping toggle and attribute calculations
 */
UCLASS()
class PRISMDMX_API UPrismDMX : public UDMXModulator {
  GENERATED_BODY()

public:
  UPrismDMX();
  /** Sets the base address for the Shifter/Override console. Format:
   * Universe.Channel */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Shifter & Multiplier",
            meta = (DisplayName = "Shifter Address (Universe.Channel)"))
  FString PixelMappingShifterAddress = TEXT("1.1");

  /** Global intensity multiplier applied during Pixel Mapping */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Shifter & Multiplier",
            meta = (DisplayName = "Intensity Multiplier"))
  float IntensityMultiplier = 2.0f;

  /** Attribute for Intensity mapping (Dimmer) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes")
  FDMXAttributeName IntensityAttribute;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes")
  FDMXAttributeName CxAttribute;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes")
  FDMXAttributeName CyAttribute;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes",
            meta = (DisplayName = "CCT Attribute"))
  FDMXAttributeName CCTAttribute;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes")
  FDMXAttributeName TintAttribute;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Fixture Attributes")
  FDMXAttributeName ColorCrossFadeAttribute;

  /** Enables detailed on-screen debugging messages */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Modulator Settings|Debug",
            meta = (DisplayName = "Enable Debug (On-Screen Messages)"))
  bool bEnableDebug = false;

  virtual void Modulate_Implementation(
      UDMXEntityFixturePatch *FixturePatch,
      const TMap<FDMXAttributeName, float> &InNormalizedAttributeValues,
      TMap<FDMXAttributeName, float> &OutNormalizedAttributeValues) override;
  virtual void ModulateMatrix_Implementation(
      UDMXEntityFixturePatch *FixturePatch,
      const TArray<FDMXNormalizedAttributeValueMap>
          &InNormalizedMatrixAttributeValues,
      TArray<FDMXNormalizedAttributeValueMap>
          &OutNormalizedMatrixAttributeValues) override;

private:
  // Caching variables for string parsing
  FString CachedAddressString = TEXT("");
  int32 CachedShifterUniverse = 1;
  int32 CachedShifterChannel = 1;

  // Shared helper function to optimally fetch Shifter and Fixture signals
  // (Pass-by-reference to avoid TArray deep copying)
  bool FetchDMXBuffers(UDMXEntityFixturePatch *FixturePatch,
                       float &OutShifterValue,
                       FDMXSignalSharedPtr &OutFixtureSignal);
};
