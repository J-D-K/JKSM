#pragma once
#include "JKSM.hpp"
#include "appstates/BaseState.hpp"

#include <memory>
#include <string>

class MessageState final : public BaseState
{
    public:
        /// @brief Creates a new message to display.
        MessageState(BaseState *creatingState, std::string_view message);

        /// @brief Required destructor.
        ~MessageState() {};

        static inline std::shared_ptr<MessageState> create(BaseState *creatingState, std::string_view message)
        {
            return std::make_shared<MessageState>(creatingState, message);
        }

        static inline std::shared_ptr<MessageState> create_and_push(BaseState *creatingState, std::string_view message)
        {
            auto newState = std::make_shared<MessageState>(creatingState, message);
            JKSM::PushState(newState);
            return newState;
        }

        /// @brief Update override.
        void update() override;

        /// @brief Draw top override.
        void draw_top(SDL_Surface *target) override;

        /// @brief Draw bottom override.
        void draw_bottom(SDL_Surface *target) override;

    private:
        /// @brief Pointer to the spawning state
        BaseState *m_creatingState{};

        /// @brief Message to display.
        std::string m_message;

        /// @brief X coordinate of OK text.
        int m_okX{};
};
