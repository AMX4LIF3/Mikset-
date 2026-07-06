#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <Core/core.hpp>
#include <../../Core/common.hpp>
#include <algorithm>

using namespace DirectX;

class Camera
{
	public:
		Camera(float x, float y, float z);
		Camera() = default; // it was giving me no default constructor exists for class hence this

		~Camera();
		
		void UpdateViewMatrix();
		XMMATRIX GetViewProjection() const;
		XMMATRIX GetRotationMatrix(bool invertPitch) const;
		XMMATRIX GetViewMatrix() const { return m_ViewMat; };
		XMMATRIX GetProjMatrix() const { return m_ProjMat; };
		void SetFov(float val);

		void TranslateZ(float amount); 
		void TranslateX(float amount); 
		void TranslateY(float ammount);

		void Rotate(float yaw, float pitch);


		XMFLOAT3 pos, forward, up, right;

	private: 
		XMMATRIX m_ViewMat, m_ProjMat;
		
		float yaw, pitch; 
		float m_fov; 
	

};

