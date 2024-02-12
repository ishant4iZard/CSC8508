#include "WindowsLevelLoader.h"
#include <filesystem>
#include <Assets.h>
namespace fs = std::filesystem;

WindowsLevelLoader::WindowsLevelLoader()
{
    LoadLevelFiles();
}

void WindowsLevelLoader::LoadLevelFiles()
{
    std::string directoryPath = NCL::Assets::ASSETROOT +  "LevelData/";

    try {
        int tempFileNum = 0;

        for (const auto& tempEntry : fs::directory_iterator(directoryPath))
            if (fs::is_regular_file(tempEntry.path()))
                levelFileNameList[tempFileNum++] = directoryPath + tempEntry.path().filename().string();

        // Print the file names
        for (size_t i = 0; i < (int)GameLevelNumber::MAX_LEVEL; i++)  std::cout << levelFileNameList[i] << std::endl;
    }   
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }
}

std::string WindowsLevelLoader::GetLevelFilePath(GameLevelNumber inLevelNum)
{
    if (inLevelNum != GameLevelNumber::MAX_LEVEL) return levelFileNameList[(int)inLevelNum];
    return "";
}
