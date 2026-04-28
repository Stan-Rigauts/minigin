#include <stdexcept>
#include <Windows.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <filesystem>
#include "Font.h"

TTF_Font* dae::Font::GetFont() const {
	return m_font;
}

dae::Font::Font(const std::string& fullPath, float size)
    : m_font(nullptr)
{
    OutputDebugStringA("=== FONT LOAD START ===\n");
    OutputDebugStringA(fullPath.c_str());
    OutputDebugStringA("\n");

    std::string exists =
        std::filesystem::exists(fullPath) ? "YES\n" : "NO\n";

    OutputDebugStringA("Exists: ");
    OutputDebugStringA(exists.c_str());

    m_font = TTF_OpenFont(fullPath.c_str(), size);

    if (!m_font)
    {
        OutputDebugStringA("TTF_OpenFont FAILED\n");
        OutputDebugStringA(SDL_GetError());
        OutputDebugStringA("\n");

        throw std::runtime_error("Font load failed: " + fullPath);
    }

    OutputDebugStringA("FONT LOADED OK\n");
}

dae::Font::~Font()
{
	TTF_CloseFont(m_font);
}
