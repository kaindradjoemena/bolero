// core/serial.hpp

#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace blr::core
{


class Scene;

class Serial
{
public:
    Serial() = delete;
    ~Serial() = default;

    Serial(const Serial&) = delete;
    Serial& operator=(const Serial&) = delete;

    Serial(Serial&& other) = delete;
    Serial& operator=(Serial&& other) = delete;


    static json Serialize(json& file, Scene& scene);

    static json Deserialize(const json&);
};


} /* namespace blr::core */