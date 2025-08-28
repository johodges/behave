#include <fstream>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "behaveRun.h"
#include "fuelModels.h"

// Define the error tolerance for double values
constexpr double error_tolerance = 1e-06;

struct TestInfo
{
    int numTotalTests = 0;
    int numFailed = 0;
    int numPassed = 0;

    const std::string reset_text_color = "\u001b[0m";
    const std::string red_text_color = "\u001b[31m";
    const std::string green_text_color = "\u001b[32m";
};

bool areClose(const double observed, const double expected, const double epsilon);

double roundToSixDecimalPlaces(const double numberToBeRounded);

void reportTestResult(TestInfo& testInfo, const string testName, const double observed, const double expected, const double epsilon);

void setSurfaceInputsForGS4LowMoistureScenario(BehaveRun& behaveRun);
void setSurfaceInputsForTwoFuelModelsLowMoistureScenario(BehaveRun& behaveRun);
void setCrownInputsLowMoistureScenario(BehaveRun& behaveRun);

void evaluateModel(BehaveRun& behaveRun, int fuelModelNumber, double moistureOneHour, double moistureTenHour, double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody,
  double windSpeed, double windDirection, double slope, double aspect, double canopyCover, double canopyHeight, double crownRatio);

std::string get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) {
        std::cerr << "could not open " << filename << std::endl;
        exit(1);
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

int main()
{
    FuelModels fuelModels;
    SpeciesMasterTable mortalitySpeciesTable;
    MoistureScenarios moistureScenarios;
    BehaveRun behaveRun(fuelModels, mortalitySpeciesTable);
    behaveRun.setMoistureScenarios(moistureScenarios);
    
    // Set defaults
    int fuelModelNumber = 122; 
    double moistureOneHour = 3.0;
    double moistureTenHour = 4.0;
    double moistureHundredHour = 5.0;
    double moistureLiveHerbaceous = 30.0;
    double moistureLiveWoody = 60.0;
    double windSpeed = 0.0;
    double windDirection = 0;
    double slope = 0.0;
    double aspect = 0;
    double canopyCover = 0;
    double canopyHeight = 0.0;
    double crownRatio = 0.50;
    double observedSurfaceFireSpreadRate = 0.0;
    double expectedSurfaceFireSpreadRate = 0.0;
    
    // overwrite defaults found in config
    std::string contents = get_file_contents("config.yaml");
    std::istringstream f(contents);
    std::string line;
    std::string delimiter = ":";
    while (std::getline(f, line)) {
        std::string key = line.substr(0, line.find(delimiter));
        std::string value = line.substr(line.find(delimiter)+2,line.length());
        if ( key == std::string("fuelModelNumber") ) { fuelModelNumber = std::stod(value); }
        if ( key == std::string("moistureOneHour") ) { moistureOneHour = std::stod(value); }
        if ( key == std::string("moistureTenHour") ) { moistureTenHour = std::stod(value); }
        if ( key == std::string("moistureHundredHour") ) { moistureHundredHour = std::stod(value); }
        if ( key == std::string("moistureLiveHerbaceous") ) { moistureLiveHerbaceous = std::stod(value); }
        if ( key == std::string("moistureLiveWoody") ) { moistureLiveWoody = std::stod(value); }
        if ( key == std::string("windSpeed") ) { windSpeed = std::stod(value); }
        if ( key == std::string("windDirection") ) { windDirection = std::stod(value); }
        if ( key == std::string("slope") ) { slope = std::stod(value); }
        if ( key == std::string("aspect") ) { aspect = std::stod(value); }
        if ( key == std::string("canopyCover") ) { canopyCover = std::stod(value); }
        if ( key == std::string("canopyHeight") ) { canopyHeight = std::stod(value); }
        if ( key == std::string("crownRatio") ) { crownRatio = std::stod(value); }
    }
    std::cout << "Starting simulation with:" << std::endl;
    std::cout << "  fuelModelNumber:" + std::to_string(fuelModelNumber) << std::endl;
    std::cout << "  moistureOneHour:" + std::to_string(moistureOneHour) << std::endl;
    std::cout << "  moistureTenHour:" + std::to_string(moistureTenHour) << std::endl;
    std::cout << "  moistureHundredHour:" + std::to_string(moistureHundredHour) << std::endl;
    std::cout << "  moistureLiveHerbaceous:" + std::to_string(moistureLiveHerbaceous) << std::endl;
    std::cout << "  moistureLiveWoody:" + std::to_string(moistureLiveWoody) << std::endl;
    std::cout << "  windSpeed:" + std::to_string(windSpeed) << std::endl;
    std::cout << "  windDirection:" + std::to_string(windDirection) << std::endl;
    std::cout << "  slope:" + std::to_string(slope) << std::endl;
    std::cout << "  aspect:" + std::to_string(aspect) << std::endl;
    std::cout << "  canopyCover:" + std::to_string(canopyCover) << std::endl;
    std::cout << "  canopyHeight:" + std::to_string(canopyHeight) << std::endl;
    std::cout << "  crownRatio:" + std::to_string(crownRatio) << std::endl;
    //std::cout << contents;
    

    evaluateModel(behaveRun,fuelModelNumber,moistureOneHour,moistureTenHour,moistureHundredHour,moistureLiveHerbaceous,moistureLiveWoody,
      windSpeed,windDirection,slope,aspect,canopyCover,canopyHeight,crownRatio);

}


void evaluateModel(BehaveRun& behaveRun, int fuelModelNumber, double moistureOneHour, double moistureTenHour, double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody,
  double windSpeed, double windDirection, double slope, double aspect, double canopyCover, double canopyHeight, double crownRatio)
{
    
    FractionUnits::FractionUnitsEnum moistureUnits = FractionUnits::Percent;
    //TwoFuelModelsMethod::TwoFuelModelsMethodEnum  twoFuelModelsMethod = TwoFuelModelsMethod::TwoDimensional;
    WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode = WindHeightInputMode::TwentyFoot;
    SpeedUnits::SpeedUnitsEnum windSpeedUnits = SpeedUnits::MilesPerHour;
    WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode = WindAndSpreadOrientationMode::RelativeToNorth;
    FractionUnits::FractionUnitsEnum firstFuelModelCoverageUnits = FractionUnits::Percent;
    SlopeUnits::SlopeUnitsEnum slopeUnits = SlopeUnits::Percent;
    FractionUnits::FractionUnitsEnum canopyCoverUnits = FractionUnits::Percent;
    LengthUnits::LengthUnitsEnum canopyHeightUnits = LengthUnits::Feet;
    FractionUnits::FractionUnitsEnum crownRatioUnits = FractionUnits::Percent;
    FractionUnits::FractionUnitsEnum canopyUnits = FractionUnits::Percent;
    SurfaceAreaToVolumeUnits::SurfaceAreaToVolumeUnitsEnum savrUnits = SurfaceAreaToVolumeUnits::SquareFeetOverCubicFeet;
    DensityUnits::DensityUnitsEnum densityUnits = DensityUnits::PoundsPerCubicFoot;
    HeatSourceAndReactionIntensityUnits::HeatSourceAndReactionIntensityUnitsEnum heatSourceUnits = HeatSourceAndReactionIntensityUnits::BtusPerSquareFootPerMinute;
    HeatSinkUnits::HeatSinkUnitsEnum heatSinkUnits = HeatSinkUnits::BtusPerCubicFoot;
    LoadingUnits::LoadingUnitsEnum loadingUnits = LoadingUnits::TonsPerAcre;
    // Observed and expected output
    double surfaceFireSpreadRate = 0.0;
    double reactionIntensity = 0.0;
    double directionOfMaxSpread = 0.0;
    double midflameWindSpeed = 0.0;
    double windAdjustmentFactor = 0.0;
    double effectiveWindSpeed = 0.0;
    double windSpeedLimit = 0.0;
    double characteristicDeadFuelMoisture = 0.0;
    double characteristicLiveFuelMoisture = 0.0;
    double liveFuelMoistureOfExtinction = 0.0;
    double characteristicSAVR = 0.0;
    double bulkDensity = 0.0;
    double packingRatio = 0.0;
    double relativePackingRatio = 0.0;
    double deadFuelReactionIntensity = 0.0;
    double liveFuelReactionIntensity = 0.0;
    double windFactor = 0.0;
    double slopeFactor = 0.0;
    double heatSource = 0.0;
    double heatSink = 0.0;
    double totalDeadHerbaceousFuelLoad = 0.0;
    double totalLiveHerbaceousFuelLoad = 0.0;
    double totalLiveFuelLoad = 0.0;
    double totalDeadFuelLoad = 0.0;

    behaveRun.surface.updateSurfaceInputs(fuelModelNumber, moistureOneHour, moistureTenHour, moistureHundredHour, moistureLiveHerbaceous,
        moistureLiveWoody, moistureUnits, windSpeed, windSpeedUnits, windHeightInputMode, windDirection, windAndSpreadOrientationMode,
        slope, slopeUnits, aspect, canopyCover, canopyUnits, canopyHeight, canopyHeightUnits, crownRatio, crownRatioUnits);
    
    behaveRun.surface.doSurfaceRunInDirectionOfMaxSpread();
    
    surfaceFireSpreadRate = behaveRun.surface.getSpreadRate(SpeedUnits::ChainsPerHour);
    reactionIntensity = behaveRun.surface.getReactionIntensity(heatSourceUnits);
    directionOfMaxSpread = behaveRun.surface.getDirectionOfMaxSpread();
    midflameWindSpeed = behaveRun.surface.getMidflameWindspeed(windSpeedUnits);
    windAdjustmentFactor = behaveRun.surface.getWindAdjustmentFactor();
    //effectiveWindSpeed = behaveRun.surface.getEffectiveWindSpeed();
    effectiveWindSpeed = SpeedUnits::fromBaseUnits(behaveRun.surface.getEffectiveWindSpeed(), windSpeedUnits);
    windSpeedLimit = SpeedUnits::fromBaseUnits(behaveRun.surface.getWindSpeedLimit(), windSpeedUnits);
    characteristicDeadFuelMoisture = behaveRun.surface.getCharacteristicMoistureByLifeState(FuelLifeState::Dead, moistureUnits);
    characteristicLiveFuelMoisture = behaveRun.surface.getCharacteristicMoistureByLifeState(FuelLifeState::Live, moistureUnits);
    liveFuelMoistureOfExtinction = behaveRun.surface.getLiveFuelMoistureOfExtinction(moistureUnits);
    characteristicSAVR = behaveRun.surface.getCharacteristicSAVR(savrUnits);
    bulkDensity = behaveRun.surface.getBulkDensity(densityUnits);
    packingRatio = behaveRun.surface.getPackingRatio();
    relativePackingRatio = behaveRun.surface.getRelativePackingRatio();
    deadFuelReactionIntensity = behaveRun.surface.getSurfaceFireReactionIntensityForLifeState(FuelLifeState::Dead);
    liveFuelReactionIntensity = behaveRun.surface.getSurfaceFireReactionIntensityForLifeState(FuelLifeState::Live);
    windFactor = behaveRun.surface.getWindFactor();
    slopeFactor = behaveRun.surface.getSlopeFactor();
    heatSource = behaveRun.surface.getHeatSource(heatSourceUnits);
    heatSink = behaveRun.surface.getHeatSink(heatSinkUnits);
    totalDeadHerbaceousFuelLoad = behaveRun.surface.getTotalDeadHerbaceousFuelLoad(loadingUnits);
    totalLiveHerbaceousFuelLoad = behaveRun.surface.getTotalLiveHerbaceousFuelLoad(loadingUnits);
    
    
    totalLiveFuelLoad = behaveRun.surface.getTotalLiveFuelLoad(loadingUnits);
    totalDeadFuelLoad = behaveRun.surface.getTotalDeadFuelLoad(loadingUnits);
    
    std::cout << "\n";
    std::cout << "Rate of spread:\t\t\t\t\t" + std::to_string(surfaceFireSpreadRate) + " ch/h\n";
    std::cout << "Reaction Intensity:\t\t\t\t" + std::to_string(reactionIntensity) + " Btus/ft2/min\n";
    std::cout << "Surface Fire Dir of Max Spread (from north):\t" + std::to_string(directionOfMaxSpread) + " deg\n";
    std::cout << "Midflame Wind Speed:\t\t\t\t" + std::to_string(midflameWindSpeed) + " mi/h\n";
    std::cout << "Wind Adjustment Factor:\t\t\t\t" + std::to_string(windAdjustmentFactor) + " \n";
    std::cout << "Effective Wind Speed:\t\t\t\t" + std::to_string(effectiveWindSpeed) + " mi/h\n";
    std::cout << "Surface Fire Effective Wind Speed Limit:\t" + std::to_string(windSpeedLimit) + " mi/h\n";
    std::cout << "Surface Fire Effective Wind Exceeded?:\t\n";
    std::cout << "Characteristic Dead Fuel Moisture:\t\t" + std::to_string(characteristicDeadFuelMoisture) + " %\n";
    std::cout << "Characteristic Live Fuel Moisture:\t\t" + std::to_string(characteristicLiveFuelMoisture) + " %\n";
    std::cout << "Live Fuel Moisture of Extinction:\t\t" + std::to_string(liveFuelMoistureOfExtinction) + " %\n";
    std::cout << "Characteristic SA/V:\t\t\t\t" + std::to_string(characteristicSAVR) + " ft2/ft3\n";
    std::cout << "Bulk Density:\t\t\t\t\t" + std::to_string(bulkDensity) + " lb/ft3\n";
    std::cout << "Packing Ratio:\t\t\t\t\t" + std::to_string(packingRatio) + " \n";
    std::cout << "Relative Packing Ratio:\t\t\t\t" + std::to_string(relativePackingRatio) + " \n";
    std::cout << "Dead Fuel Reaction Intensity:\t\t\t" + std::to_string(deadFuelReactionIntensity) + " \n";
    std::cout << "Live Fuel Reaction Intensity:\t\t\t" + std::to_string(liveFuelReactionIntensity) + " \n";
    std::cout << "Surface Fire Wind Factor:\t\t\t" + std::to_string(windFactor) + " \n";
    std::cout << "Slope Factor:\t\t\t\t\t" + std::to_string(slopeFactor) + " \n";
    std::cout << "Heat Source:\t\t\t\t\t" + std::to_string(heatSource) + " Btu/ft2/min\n";
    std::cout << "Heat Sink:\t\t\t\t\t" + std::to_string(heatSink) + " Btu/ft3\n";
    std::cout << "Flame Residence Time:\t\n";
    std::cout << "Fuel Load Transfer Portion:\t\n";
    std::cout << "Dead Herbaceous Fuel Load:\t\t\t" + std::to_string(totalDeadHerbaceousFuelLoad) + " ton/ac\n";
    std::cout << "Live Fuel Load Remainder:\t\t\t" + std::to_string(totalLiveHerbaceousFuelLoad) + " ton/ac\n";
    std::cout << "Total Dead Fuel Load:\t\t\t\t" + std::to_string(totalDeadFuelLoad) + " ton/ac\n";
    std::cout << "Total Live Fuel Load:\t\t\t\t" + std::to_string(totalLiveFuelLoad) + " ton/ac\n";
    std::cout << "Dead Fuel Load Portion:\t\t\t\t" + std::to_string(100*totalDeadFuelLoad/(totalLiveFuelLoad+totalDeadFuelLoad)) + " %\n";
    std::cout << "Live Fuel Load Portion:\t\t\t\t" + std::to_string(100*totalLiveFuelLoad/(totalLiveFuelLoad+totalDeadFuelLoad)) + " %\n";
    std::cout << "\n";
    
    std::cout << "Finished evaluation, single fuel model\n\n";
}







