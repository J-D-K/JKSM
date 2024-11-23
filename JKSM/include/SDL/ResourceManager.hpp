#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace SDL
{
    class Surface;
    class Font;

    using SharedSurface = std::shared_ptr<SDL::Surface>;
    using SharedFont = std::shared_ptr<SDL::Font>;

    template <typename Type>
    class ResourceManager
    {
        public:
            // No copying.
            ResourceManager(const ResourceManager &) = delete;
            ResourceManager(ResourceManager &&) = delete;
            ResourceManager &operator=(const ResourceManager &) = delete;
            ResourceManager &operator=(ResourceManager &&) = delete;

            template <typename... Args>
            static std::shared_ptr<Type> CreateLoadResource(std::string_view ResourceName, Args... Arguments)
            {
                std::shared_ptr<Type> NewResource = nullptr;

                ResourceManager &Instance = GetInstance();

                if (Instance.m_ResourceMap.find(ResourceName.data()) != Instance.m_ResourceMap.end() &&
                    !Instance.m_ResourceMap.at(ResourceName.data()).expired())
                {
                    NewResource = Instance.m_ResourceMap.at(ResourceName.data()).lock();
                }
                else
                {
                    NewResource = std::make_shared<Type>(Arguments...);

                    Instance.m_ResourceMap[ResourceName.data()] = NewResource;
                }
                return NewResource;
            }

        private:
            // No constructing.
            ResourceManager(void) = default;

            static ResourceManager &GetInstance(void)
            {
                static ResourceManager Instance;
                return Instance;
            }
            // Resource map.
            static inline std::unordered_map<std::string, std::weak_ptr<Type>> m_ResourceMap;
    };
    using SurfaceManager = SDL::ResourceManager<SDL::Surface>;
    using FontManager = SDL::ResourceManager<SDL::Font>;
} // namespace SDL
