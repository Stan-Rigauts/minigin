#pragma once
#include "Component.h"
#include "TileType.h"
#include "LevelLoader.h"
#include <vector>

namespace dae
{
    class GridComponent final : public Component
    {
    public:
        GridComponent(GameObject& owner, const MapInfo& mapInfo, int tileSize, int offsetX, int offsetY)
            : Component(owner)
            , m_Grid(mapInfo.grid)
            , m_Cols(mapInfo.cols)
            , m_Rows(mapInfo.rows)
            , m_TileSize(tileSize)
            , m_OffsetX(offsetX)
            , m_OffsetY(offsetY)
        {
        }

        bool IsWall(float worldX, float worldY) const
        {
            int col = static_cast<int>((worldX - m_OffsetX) / m_TileSize);
            int row = static_cast<int>((worldY - m_OffsetY) / m_TileSize);
            return GetTile(col, row) == TileType::Wall;
        }

        TileType GetTile(int col, int row) const
        {
            if (row < 0 || row >= m_Rows) return TileType::Wall;
            if (col < 0 || col >= m_Cols) return TileType::Wall;
            return m_Grid[row][col];
        }

        TileType GetTileAtWorld(float worldX, float worldY) const
        {
            int col = static_cast<int>((worldX - m_OffsetX) / m_TileSize);
            int row = static_cast<int>((worldY - m_OffsetY) / m_TileSize);
            return GetTile(col, row);
        }

        void SetTile(int col, int row, TileType type)
        {
            if (row < 0 || row >= m_Rows) return;
            if (col < 0 || col >= m_Cols) return;
            m_Grid[row][col] = type;
        }

        void ClearTileAtWorld(float worldX, float worldY)
        {
            int col = static_cast<int>((worldX - m_OffsetX) / m_TileSize);
            int row = static_cast<int>((worldY - m_OffsetY) / m_TileSize);
            SetTile(col, row, TileType::Empty);
        }

        int GetTileSize() const { return m_TileSize; }
        int GetOffsetX()  const { return m_OffsetX; }
        int GetOffsetY()  const { return m_OffsetY; }
        int GetCols()     const { return m_Cols; }
        int GetRows()     const { return m_Rows; }

    private:
        std::vector<std::vector<TileType>> m_Grid;
        int m_Cols{};
        int m_Rows{};
        int m_TileSize{};
        int m_OffsetX{};
        int m_OffsetY{};
    };
}