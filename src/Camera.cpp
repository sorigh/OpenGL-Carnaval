#include "Camera.hpp"

namespace gps {
    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUpDirection) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }
	
    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO
        return glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction) {
        case gps::MOVE_FORWARD:
            this->cameraPosition += this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_BACKWARD:
            this->cameraPosition -= this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_RIGHT:
            this->cameraPosition += glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection)) * speed;
            break;
        case gps::MOVE_LEFT:
            this->cameraPosition -= glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection)) * speed;
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        this->cameraFrontDirection = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch)));

        this->cameraFrontDirection = glm::normalize(this->cameraFrontDirection);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }
}