#include "scene/Transform.h"

namespace plunger {

void Transform::rebuildLocalMatrix()
{
    localMatrix = multiply(
        translate(position),
        multiply(
            rotateY(rotation.y),
            multiply(rotateX(rotation.x), multiply(rotateZ(rotation.z), plunger::scale(scaleFactor)))));
    dirty = true;
}

} // namespace plunger