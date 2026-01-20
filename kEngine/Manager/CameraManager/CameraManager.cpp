#include "CameraManager.h"

CameraManager::CameraManager() {
    defaultCamera_ = new Camera();
    activeCamera_ = defaultCamera_;
}

CameraManager::~CameraManager() {
    delete defaultCamera_;
}

Camera* CameraManager::CreateCamera() {
    Camera* cam = new Camera();
    return cam;
}

DebugCamera* CameraManager::CreateDebugCamera(kEngine* kEngine) {
	DebugCamera* cam = new DebugCamera(kEngine);
	return cam;
}

void CameraManager::DestroyCamera(Camera* cam) {
    if (activeCamera_ == cam)
        activeCamera_ = defaultCamera_;
    delete cam;
}

void CameraManager::SetActiveCamera(Camera* cam) {
    activeCamera_ = cam ? cam : defaultCamera_;
}

void CameraManager::ResetActiveCamera() {
    activeCamera_ = defaultCamera_;
}

Camera* CameraManager::GetActiveCamera() const {
    return activeCamera_ ? activeCamera_ : defaultCamera_;
}
