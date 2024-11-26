#pragma once
#include <json-c/json.h>
#include <memory>

namespace JSON
{
    using Object = std::unique_ptr<json_object, decltype(&json_object_put)>;
    template <typename... Args>
    JSON::Object NewObject(json_object *(*Function)(Args...), Args... Arguments)
    {
        return JSON::Object((*Function)(Arguments...), json_object_put);
    }
} // namespace JSON
