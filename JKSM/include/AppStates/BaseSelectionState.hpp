#pragma once
#include "AppStates/AppState.hpp"
#include "Data/Data.hpp"
#include <vector>

class BaseSelectionState : public AppState
{
    public:
        BaseSelectionState(Data::SaveDataType SaveType) : m_SaveType(SaveType) {};
        ~BaseSelectionState() {};
        void Update(void) = 0;
        void DrawTop(SDL_Surface *Target) = 0;
        void DrawBottom(SDL_Surface *Target) = 0;

        // This renders the information on the bottom for both states.
        void DrawTitleInformation(SDL_Surface *Target, const Data::TitleData *Data);

        // Both states need a refresh function.
        virtual void Refresh(void) = 0;

    protected:
        // Save data type
        Data::SaveDataType m_SaveType;
};
