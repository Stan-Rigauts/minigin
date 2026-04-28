#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "TileType.h"

namespace dae
{
    struct MapInfo
    {
        std::vector<std::vector<TileType>> grid;
        int cols{};
        int rows{};
    };

    class LevelLoader
    {
    public:
        static MapInfo LoadMap(const std::string& mapFile)
        {
            std::vector<std::string> paths = {
                mapFile,
                "Data/" + mapFile,
                "../Data/" + mapFile,
                "../../Data/" + mapFile
            };

            std::ifstream file;
            for (auto& path : paths)
            {
                file.open(path);
                if (file.is_open())
                    break;
            }

            if (!file.is_open())
                throw std::runtime_error("Could not open map file: " + mapFile);

            std::vector<std::vector<TileType>> grid;
            std::string line;
            int maxCols = 0;

            while (std::getline(file, line))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                maxCols = std::max(maxCols, static_cast<int>(line.size()));

                std::vector<TileType> row;
                for (char c : line)
                {
                    switch (c)
                    {
                    case '#': row.push_back(TileType::Wall);        break;
                    case '.': row.push_back(TileType::Pellet);      break;
                    case 'o': row.push_back(TileType::PowerPellet); break;
                    case '-': row.push_back(TileType::GhostDoor);   break;
                    case 'P': row.push_back(TileType::PlayerSpawn); break;
                    case 'G': row.push_back(TileType::GhostSpawn);  break;
                    case 'e':  row.push_back(TileType::Empty);      break;
                    default:  row.push_back(TileType::Empty);       break;
                    }
                }
                grid.push_back(std::move(row));
            }

            for (auto& r : grid)
                while (static_cast<int>(r.size()) < maxCols)
                    r.push_back(TileType::Empty);

            MapInfo info;
            info.grid = std::move(grid);
            info.cols = maxCols;
            info.rows = static_cast<int>(info.grid.size());
            return info;
        }
    };
}