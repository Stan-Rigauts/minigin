#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>

namespace dae
{
    struct HighScoreEntry
    {
        std::string name;
        int score{};
    };

    class HighScoreManager
    {
    public:
        static HighScoreManager& GetInstance()
        {
            static HighScoreManager instance;
            return instance;
        }

        void Load(const std::string& path = "highscores.txt")
        {
            if (m_LoadThread.joinable())
                m_LoadThread.join();

            m_LoadDone = false;
            m_LoadThread = std::thread([this, path]()
                {
                    std::vector<HighScoreEntry> entries;
                    std::ifstream file(path);
                    if (file.is_open())
                    {
                        std::string name; int score;
                        while (file >> name >> score)
                            entries.push_back({ name, score });
                    }

                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Entries = std::move(entries);
                    Sort();
                    m_LoadDone = true;
                });
        }

        void WaitForLoad()
        {
            if (m_LoadThread.joinable())
                m_LoadThread.join();
        }

        bool IsLoadDone() const { return m_LoadDone; }

        void Save(const std::string& path = "highscores.txt")
        {
            WaitForLoad();
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::ofstream file(path);
            for (auto& e : m_Entries)
                file << e.name << " " << e.score << "\n";
        }

        void Submit(const std::string& name, int score)
        {
            WaitForLoad();
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Entries.push_back({ name, score });
            Sort();
            if (m_Entries.size() > MAX_ENTRIES)
                m_Entries.resize(MAX_ENTRIES);
            std::thread([this, path = std::string("highscores.txt")]() { Save(path); }).detach();
        }

        bool IsHighScore(int score)
        {
            WaitForLoad();
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (static_cast<int>(m_Entries.size()) < MAX_ENTRIES) return true;
            return score > m_Entries.back().score;
        }

        const std::vector<HighScoreEntry> GetEntries()
        {
            WaitForLoad();
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Entries;
        }

        ~HighScoreManager()
        {
            if (m_LoadThread.joinable())
                m_LoadThread.join();
        }

    private:
        void Sort()
        {
            std::sort(m_Entries.begin(), m_Entries.end(),
                [](const HighScoreEntry& a, const HighScoreEntry& b)
                { return a.score > b.score; });
        }

        static constexpr int MAX_ENTRIES = 5;
        std::vector<HighScoreEntry> m_Entries;
        mutable std::mutex m_Mutex;
        std::thread m_LoadThread;
        std::atomic<bool> m_LoadDone{ false };
    };
}