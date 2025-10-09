#include "Core/Math/Common.h"

HS_NS_BEGIN

bool Math::EpsilonEqual(float lhs, float rhs)
{
    return std::abs(rhs - lhs) <= HS_FLT_EPSILON;
}

HS_NS_END

namespace glm
{
const glm::vec2 zero2 = { 0.0f, 0.0f };
const glm::vec2 up2 = { 0.0f, 1.0f };
const glm::vec2 right2 = { 1.0f, 0.0f };

const glm::vec3 zero3 = { 0.0f, 0.0f, 0.0f };
const glm::vec3 up3 = { 0.0f, 1.0f, 0.0f };
const glm::vec3 right3 = { 1.0f, 0.0f, 0.0f };
const glm::vec3 front3 = { 0.0f, 0.0f, 1.0f };

const glm::vec4 zero4 = {0.0f, 0.0f, 0.0f, 0.0f};
const glm::vec4 up4 = {0.0f, 1.0f, 0.0f, 0.0f};
const glm::vec4 right4 = {0.0f, 0.0f, 1.0f, 0.0f};
const glm::vec4 front4 = { 1.0f, 0.0f, 0.0f, 0.0f };

const glm::mat4 identity4 = glm::mat4(1.0f);
const glm::mat3 identity3 = glm::mat3(1.0f);
const glm::mat2 identity2 = glm::mat2(1.0f);

}