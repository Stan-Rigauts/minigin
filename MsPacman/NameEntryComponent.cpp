#include "NameEntryComponent.h"
#include "GameObject.h"
#include "TextComponent.h"

namespace dae
{
    NameEntryComponent::NameEntryComponent(GameObject& owner, int score,
        TextComponent* textComponent,
        std::function<void(const std::string&, int)> onConfirm)
        : Component(owner)
        , m_Score(score)
        , m_pTextComponent(textComponent)                    
        , m_OnConfirm(std::move(onConfirm))
    {
        m_Letters.fill(0);
        UpdateDisplayText();
    }

    void NameEntryComponent::MoveLeft()
    {
        if (!*m_pActive) return;
        m_Letters[m_CursorPos] = (m_Letters[m_CursorPos] - 1 + ALPHA_LEN) % ALPHA_LEN;
        UpdateDisplayText();
    }

    void NameEntryComponent::MoveRight()
    {
        if (!*m_pActive) return;
        m_Letters[m_CursorPos] = (m_Letters[m_CursorPos] + 1) % ALPHA_LEN;
        UpdateDisplayText();
    }

    void NameEntryComponent::Confirm()
    {
        if (!*m_pActive) return;
        if (m_CursorPos < MAX_LETTERS - 1)
        {
            ++m_CursorPos;
            UpdateDisplayText();
        }
        else if (m_CursorPos == MAX_LETTERS - 1) 
        {
            std::string name;
            for (int i = 0; i < MAX_LETTERS; ++i)
                name += ALPHABET[m_Letters[i]];
            if (m_OnConfirm)
            {
                *m_pActive = false;
                auto cb = std::move(m_OnConfirm);  
                m_OnConfirm = nullptr;
                cb(name, m_Score);
            }
        }
    }

    void NameEntryComponent::Back()
    {
        if (!*m_pActive) return;
        if (m_CursorPos > 0)
        {
            --m_CursorPos;
            UpdateDisplayText();
        }
    }

    void NameEntryComponent::UpdateDisplayText()
    {
        std::string display;
        for (int i = 0; i < MAX_LETTERS; ++i)
        {
            if (i == m_CursorPos)
                display += '[';
            display += ALPHABET[m_Letters[i]];
            if (i == m_CursorPos)
                display += ']';
            if (i < MAX_LETTERS - 1)
                display += ' ';
        }

        if (m_pTextComponent)
            m_pTextComponent->SetText(display);
    }

    void NameEntryComponent::Update(float) {}
}