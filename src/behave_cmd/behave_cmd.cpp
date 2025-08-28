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
    TwoFuelModelsMethod::TwoFuelModelsMethodEnum  twoFuelModelsMethod = TwoFuelModelsMethod::TwoDimensional;
    WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode = WindHeightInputMode::TwentyFoot;
    SpeedUnits::SpeedUnitsEnum windSpeedUnits = SpeedUnits::MilesPerHour;
    WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode = WindAndSpreadOrientationMode::RelativeToNorth;
    FractionUnits::FractionUnitsEnum firstFuelModelCoverageUnits = FractionUnits::Percent;
    SlopeUnits::SlopeUnitsEnum slopeUnits = SlopeUnits::Percent;
    FractionUnits::FractionUnitsEnum canopyCoverUnits = FractionUnits::Percent;
    LengthUnits::LengthUnitsEnum canopyHeightUnits = LengthUnits::Feet;
    FractionUnits::FractionUnitsEnum crownRatioUnits = FractionUnits::Percent;
    FractionUnits::FractionUnitsEnum canopyUnits = FractionUnits::Percent;
    // Observed and expected output
    double observedSurfaceFireSpreadRate = 0.0;
    double expectedSurfaceFireSpreadRate = 0.0;

    behaveRun.surface.updateSurfaceInputs(fuelModelNumber, moistureOneHour, moistureTenHour, moistureHundredHour, moistureLiveHerbaceous,
        moistureLiveWoody, moistureUnits, windSpeed, windSpeedUnits, windHeightInputMode, windDirection, windAndSpreadOrientationMode,
        slope, slopeUnits, aspect, canopyCover, canopyUnits, canopyHeight, canopyHeightUnits, crownRatio, crownRatioUnits);
    
    behaveRun.surface.doSurfaceRunInDirectionOfMaxSpread();
    
    observedSurfaceFireSpreadRate = behaveRun.surface.getSpreadRate(SpeedUnits::ChainsPerHour);
    
    std::cout << "Rate of spread:";
    std::cout << observedSurfaceFireSpreadRate;
    std::cout << "\n";
    
    std::cout << "Finished evaluation, single fuel model\n\n";
}







