#include "EditorCamera.h"

#include "Core/Log.h"
#include "Core/Input.h"

#include <glfw/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <iostream>


EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
	: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip),
	Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
{
	UpdateView();
}

void EditorCamera::UpdateProjection()
{
	m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
	Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
}

void EditorCamera::UpdateView()
{
	m_Position = CalculatePosition();
	glm::quat orientation = GetOrientation();
	m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
	// current configuration spins camera around.
	// Inversing camera view will have the effect of orbiting the camera around the focal point.
	m_ViewMatrix = glm::inverse(m_ViewMatrix);
}

void EditorCamera::OnMouseScroll(float xOffset, float yOffset) {
	float delta = yOffset * 0.5f;
	MouseZoom(delta);
	UpdateView();
}

std::pair<float, float> EditorCamera::PanSpeed() const
{
	float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

	float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	return { xFactor, yFactor };
}

float EditorCamera::RotationSpeed() const
{
	return 0.8f;
}

float EditorCamera::ZoomSpeed() const
{
	float distance = m_Distance * 0.2f;
	distance = std::max(distance, 0.0f); // distance needs to be positive
	float speed = distance * distance;
	speed = std::min(speed, 100.0f);
	return speed;
}

void EditorCamera::OnUpdate(float ts)
{
	if (Input::Get()->IsKeyPressed(GLFW_KEY_LEFT_ALT))
	{
		const glm::vec2& mouse = Input::Get()->GetMouseCursorPosition();
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if (Input::Get()->IsMouseButtonPressed(MouseButton::Middle))
			MousePan(delta);
		else if (Input::Get()->IsMouseButtonPressed(MouseButton::Left))
			MouseRotate(delta);
		else if (Input::Get()->IsMouseButtonPressed(MouseButton::Right))
			MouseZoom(delta.y);
	}
	
	UpdateView();
	GetForwardDirection();
}

void EditorCamera::MousePan(const glm::vec2& delta)
{
	auto [xSpeed, ySpeed] = PanSpeed();
	m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
	m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
}

void EditorCamera::MouseRotate(const glm::vec2& delta)
{
	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	m_Yaw += yawSign * delta.x * RotationSpeed();
	m_Pitch += delta.y * RotationSpeed();
}

void EditorCamera::MouseZoom(float delta)
{
	m_Distance -= delta * ZoomSpeed();
	//if (m_Distance < 1.0f)
	//{
	//	m_FocalPoint += GetForwardDirection();
	//	m_Distance = 1.0f;
	//}
}

glm::vec3 EditorCamera::GetUpDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 EditorCamera::GetRightDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 EditorCamera::GetForwardDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 EditorCamera::CalculatePosition() const
{
	return m_FocalPoint - GetForwardDirection() * m_Distance;
}

glm::quat EditorCamera::GetOrientation() const
{
	return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
}