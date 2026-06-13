#pragma once
#include "Component.h"
#include <string>
#include <functional>
#include <array> 
#include <memory>

namespace dae
{
    class TextComponent;

    class NameEntryComponent final : public Component
    {
    public:
        static constexpr int MAX_LETTERS = 3;

        NameEntryComponent(GameObject& owner, int score,
            TextComponent* textComponent,                         
            std::function<void(const std::string&, int)> onConfirm);

        void Update(float delta) override;

        void MoveLeft();
        void MoveRight();
        void Confirm();
        void Back();
      
        std::shared_ptr<bool> GetActiveFlag() const { return m_pActive; }
    private:
        void UpdateDisplayText();

        int m_Score{};
        int m_CursorPos{ 0 };                    
        std::array<int, MAX_LETTERS> m_Letters{}; 
        std::function<void(const std::string&, int)> m_OnConfirm;

        static constexpr char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static constexpr int  ALPHA_LEN = 26;
        TextComponent* m_pTextComponent{ nullptr };
        std::shared_ptr<bool> m_pActive{ std::make_shared<bool>(true) };
    };
}