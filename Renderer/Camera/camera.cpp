#include "camera.hpp"

Camera::Camera(float x, float y, float z)
	: pos(x,y,z)
{
	this->forward = XMFLOAT3(0.0f, 0.0f, 1.0f); 
	this->up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT; 
	this->m_fov = XMConvertToRadians(88.0f);
	this->m_ProjMat = XMMatrixPerspectiveFovLH(m_fov, aspectRatio, 0.1f, 1000.0f);
	
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix() {
	//XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&this->forward) - XMLoadFloat3(&this->pos));
	XMVECTOR fwd = XMLoadFloat3(&this->forward);
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR r = XMVector3Normalize(XMVector3Cross(worldUp, fwd));
	XMStoreFloat3(&this->right, r);

	// calc local up vector
	XMVECTOR u = XMVector3Cross(fwd, r);
	XMStoreFloat3(&this->up, u);

	// build the matrix
	this->m_ViewMat = XMMatrixLookToLH(XMLoadFloat3(&this->pos), fwd, u);
}

XMMATRIX Camera::GetViewProjection() const {
	return this->m_ViewMat * this->m_ProjMat;
}



XMMATRIX Camera::GetRotationMatrix(bool invertPitch) const {
	if (invertPitch) return XMMatrixRotationRollPitchYaw(-this->pitch, this->yaw, 0.0f);
	else			 return XMMatrixRotationRollPitchYaw( this->pitch, this->yaw, 0.0f);
}

void Camera::TranslateZ(float ammount) {
	XMVECTOR p = XMLoadFloat3(&this->pos);
	XMVECTOR f = XMLoadFloat3(&this->forward);

	p += f * ammount; // move along the direction vector

	XMStoreFloat3(&this->pos, p);
	this->UpdateViewMatrix();
}

void Camera::TranslateX(float ammount) {
	XMVECTOR posVec = XMLoadFloat3(&this->pos);
	XMVECTOR rightVec = XMLoadFloat3(&this->right);

	posVec += rightVec * ammount; 
	XMStoreFloat3(&this->pos, posVec);
	this->UpdateViewMatrix();
}

void Camera::TranslateY(float ammount) {
	this->pos.y += ammount;
	this->UpdateViewMatrix();
}

void Camera::Rotate(float dyaw, float dpitch) { // delta yaw and pitch
	this->yaw += dyaw;
	this->pitch += dpitch;

	// clamp the bitch
	this->pitch = std::clamp(pitch, -1.55f, 1.55f);
	

	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(this->pitch, this->yaw, 0.0f);
	XMVECTOR baseForward = XMVectorSet(0, 0, 1, 0); // default forward
	XMVECTOR forward = XMVector3TransformNormal(baseForward, rotMat);

	XMStoreFloat3(&this->forward, XMVector3Normalize(forward));
	
	// calculate new forward vector
	this->UpdateViewMatrix();
}

void Camera::SetFov(float val) {
	if (val > 150) 
		MessageBoxA(NULL, "Caution: FOV Values higher than 150 WILL cause issues!..", "WOAH BUDDY", MB_OK | MB_ICONWARNING);
	
	this->m_fov = XMConvertToRadians(val);
}


Camera::~Camera() {
	// nun to see here bud 
}




