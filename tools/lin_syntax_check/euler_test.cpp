#include "engine/editor/EditorMath.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Mirrors Transform::CalcRotationMatrix: R = Ry(rot.x) * Rx(rot.y) * Rz(rot.z)
static glm::mat4 EngineRotation(glm::vec3 r) {
    return glm::rotate(glm::mat4(1.0f), r.x, glm::vec3(0,1,0))
         * glm::rotate(glm::mat4(1.0f), r.y, glm::vec3(1,0,0))
         * glm::rotate(glm::mat4(1.0f), r.z, glm::vec3(0,0,1));
}

int main() {
    srand(7);
    int failures = 0;
    for (int i = 0; i < 10000; i++) {
        glm::vec3 angles(
            ((rand()/(float)RAND_MAX) * 2 - 1) * 3.1f,          // yaw: full range
            ((rand()/(float)RAND_MAX) * 2 - 1) * 1.5f,          // pitch: inside +-pi/2
            ((rand()/(float)RAND_MAX) * 2 - 1) * 3.1f);         // roll: full range
        glm::mat4 m = EngineRotation(angles);
        // add non-uniform scale like a real model matrix
        m = m * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 2.0f, 1.3f));
        glm::vec3 out = EulerYXZFromMatrix(m);
        glm::mat4 rebuilt = EngineRotation(out);
        glm::mat4 orig = EngineRotation(angles);
        for (int c = 0; c < 3; c++)
            for (int r = 0; r < 3; r++)
                if (std::fabs(rebuilt[c][r] - orig[c][r]) > 1e-4f) { failures++; c = 3; break; }
    }
    printf(failures ? "EULER SELF-TEST FAILED: %d/10000\n" : "euler self-test OK (10000 cases)\n", failures);
    return failures != 0;
}
