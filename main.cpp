#include <filesystem>
#include <fstream>
#include <sstream>
#include <atomic>
#include <memory>
#include <mutex>
#include "OPT.h"
#include <thread>
#include <chrono>
#include <string>
#include <cctype>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "win.h"
namespace fs = std::filesystem;

// Thread synchronization
std::atomic<bool> g_running{true};
std::mutex configMutex;

std::pair<std::string, std::string> split(const std::string &str)
{
    std::pair<std::string, std::string> splitted;
    std::stringstream s(str);
    std::string ext;
    std::string directory;

    std::getline(s, ext, ':');
    std::getline(s, directory);
    return {ext, directory};
}

void lowerCase(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
                   { return std::tolower(c); });
}

bool load_organizer_configs(std::map<std::string, std::string> &configs)
{
    std::ifstream file("organizer_configs.txt");
    if (!file.is_open())
    {
        std::ofstream newFile("organizer_configs.txt");
        return newFile.is_open();
    }

    std::string linebuf;
    while (std::getline(file, linebuf))
    {
        configs.emplace(split(linebuf));
    }
    return true;
}

bool save_organizer_configs(const std::vector<std::string> &configs)
{
    std::ofstream file("organizer_configs.txt", std::ios::app);
    if (!file.is_open())
    {
        return false;
    }

    for (const auto &config : configs)
    {
        file << config << "\n";
    }
    return true;
}

bool move_file(const std::string &file_path, const std::string &destination_dir)
{
    try
    {
        fs::path source(file_path);
        fs::path destination(destination_dir);

        if (!fs::exists(source))
        {
            return false;
        }

        if (!fs::exists(destination))
        {
            if (!fs::create_directories(destination))
            {
                return false;
            }
        }

        fs::rename(source, destination / source.filename());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void monitor_directory(const std::map<std::string, std::string> &initialConfigs)
{
    // Create local copy of configs
    std::map<std::string, std::string> localConfigs = initialConfigs;
    fs::path path_to_watch(get_downloads_path());
    auto last_write_time = fs::last_write_time(path_to_watch);

    while (g_running)
    {
        // Check for config updates (once per second)
        {
            std::lock_guard<std::mutex> lock(configMutex);
            localConfigs = initialConfigs; // Update from main configs
        }

        try
        {
            auto current_write_time = fs::last_write_time(path_to_watch);
            if (last_write_time != current_write_time)
            {
                last_write_time = current_write_time;

                for (const auto &entry : fs::directory_iterator(path_to_watch))
                {
                    if (!g_running)
                        break;

                    if (entry.is_regular_file())
                    {
                        std::string ext = entry.path().extension().string();
                        lowerCase(ext);

                        if (localConfigs.find(ext) != localConfigs.end())
                        {
                            move_file(entry.path().string(), localConfigs.at(ext));
                        }
                    }
                }
            }
        }
        catch (...)
        {
            // Ignore filesystem errors
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char **argv)
{
    std::map<std::string, std::string> configs;
    if (!load_organizer_configs(configs))
    {
        return EXIT_FAILURE;
    }

    OPT opt(argc, argv);
    opt.add_argument("-a");
    opt.parse_args();

    auto values = opt["-a"];
    if (!values.empty())
    {
        if (!save_organizer_configs(values))
        {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "File Organizer");
    SetTargetFPS(60);
    GuiLoadStyle("raygui\\styles\\amber\\style_amber.rgs");
    // GUI state variables
    int textEditMode = 0;
    char newExt[32] = {0};
    char newDir[256] = {0};
    bool showAddDialog = false;
    int activeConfig = -1;
    bool windowVisible = true;
    bool ctrlPressed = false;
    // Start monitor thread with initial configs
    std::thread monitor_thread(monitor_directory, std::cref(configs));

    tool(GetWindowHandle());
    MinimizeToTray();
    while (!WindowShouldClose())
    {

        if (IsKeyPressed(KEY_ESCAPE))
            showAddDialog = false;
        if(IsKeyPressed(KEY_LEFT_CONTROL))
            ctrlPressed = true;
        if(IsKeyReleased(KEY_LEFT_CONTROL))
            ctrlPressed = false;
        if(ctrlPressed && IsKeyPressed(KEY_H))
            MinimizeToTray();
        BeginDrawing();
        Color bgColor = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));

        ClearBackground(bgColor);

        // Draw current configurations
        GuiLabel((Rectangle){20, 20, 200, 30}, "Current Configurations:");
        int yPos = 50;

        {
            std::lock_guard<std::mutex> lock(configMutex);
            for (const auto &[ext, dir] : configs)
            {
                if (GuiButton((Rectangle){20, yPos, 200, 30}, ext.c_str()))
                {
                    activeConfig = yPos;
                }
                GuiLabel((Rectangle){240, yPos, 500, 30}, dir.c_str());
                yPos += 35;
            }
        }

        // Add new configuration button
        if (GuiButton((Rectangle){20, yPos + 20, 200, 30}, "Add New Configuration"))
        {
            showAddDialog = true;
            textEditMode = 1;
            memset(newExt, 0, sizeof(newExt));
            memset(newDir, 0, sizeof(newDir));
        }

        // Add configuration dialog
        if (showAddDialog)
        {
            if (GuiWindowBox((Rectangle){20, yPos + 60, 400, 200}, "Add Configuration"))
            {
                showAddDialog = false;
                textEditMode = 0;
            }

            GuiLabel((Rectangle){30, yPos + 90, 100, 20}, "Extension:");
            if (GuiTextBox((Rectangle){30, yPos + 110, 60, 20}, newExt, sizeof(newExt) - 1, textEditMode == 1))
            {
                textEditMode = (textEditMode == 1) ? 2 : 1;
            }

            GuiLabel((Rectangle){30, yPos + 140, 100, 20}, "Directory:");
            if (GuiTextBox((Rectangle){30, yPos + 160, 250, 20}, newDir, sizeof(newDir) - 1, textEditMode == 2))
            {
                textEditMode = (textEditMode == 2) ? 1 : 2;
            }

            if (GuiButton((Rectangle){30, yPos + (260 - 40), 80, 30}, "Add"))
            {
                if (strlen(newExt) > 0 && strlen(newDir) > 0)
                {
                    {
                        std::lock_guard<std::mutex> lock(configMutex);
                        configs[newExt] = newDir;
                    }
                    save_organizer_configs({std::string(newExt) + ":" + newDir});
                    showAddDialog = false;
                    textEditMode = 0;
                }
            }
        }
        EndDrawing();
    }

    // Cleanup
    g_running = false;
    if (monitor_thread.joinable())
    {
        monitor_thread.join();
    }
    RemoveTrayIcon();
    CloseWindow();
    return EXIT_SUCCESS;
}