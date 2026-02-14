#include "Resource/Model.h"

HS_NS_BEGIN

void Model::Update()
{
    if (_isMeshDirty)
    {

    }
    if (_isMaterialDirty)
    {

    }
    if (_isTransformDirty)
    {
        _worldMatrix = glm::translate(glm::mat4(1.0f), _position);
        _worldMatrix = glm::rotate(_worldMatrix, _rotation.y, glm::vec3(0, 1, 0));
        _worldMatrix = glm::rotate(_worldMatrix, _rotation.x, glm::vec3(1, 0, 0));
        _worldMatrix = glm::rotate(_worldMatrix, _rotation.z, glm::vec3(0, 0, 1));
        _worldMatrix = glm::scale(_worldMatrix, _scale);
        _inverseWorldMatrix = glm::inverse(_worldMatrix);
        _isTransformDirty = false;
    }
}

HS_NS_END
