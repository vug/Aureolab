#pragma once

#include <glm/glm.hpp>

#include <utility>

class Camera {
public:
	Camera() = default;
	Camera(const glm::mat4& projection)
		: Projection(projection) {}
	virtual ~Camera() = default;

	const glm::mat4& GetProjection() const { return Projection; }
protected:
	glm::mat4 Projection = glm::mat4(1.0f);
};


/*
* Taken from https ://github.com/TheCherno/Hazel/commit/96c57dadf5c3616ba3613034dfb63d3918590359 and modified
* Camera is defined by a focal point, a distance from a focal point, and two angles describing the orientation towards the focal point (yaw, pitch).
* The position of the camera/eye is computed from these parameters.
* Including FOV and aspect ratio camera can compute the View and Projection matrices. 
*/ 
class EditorCamera : public Camera {
public:
	EditorCamera() = default;
	EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

	void OnUpdate(float ts);

	inline float GetDistance() const { return m_Distance; }
	inline void SetDistance(float distance) { m_Distance = distance; }

	inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	glm::mat4 GetViewProjection() const { return Projection * m_ViewMatrix; }

	glm::vec3 GetUpDirection() const;
	glm::vec3 GetRightDirection() const;
	glm::vec3 GetForwardDirection() const;
	const glm::vec3& GetPosition() const { return m_Position; }
	glm::quat GetOrientation() const;
	glm::vec3 GetFocalPoint() const { return m_FocalPoint; }

	float GetPitch() const { return m_Pitch; }
	float GetYaw() const { return m_Yaw; }
private:
	void UpdateProjection();
	void UpdateView();

	void OnMouseScroll(float xOffset, float yOffset);

	void MousePan(const glm::vec2& delta);
	void MouseRotate(const glm::vec2& delta);
	void MouseZoom(float delta);

	glm::vec3 CalculatePosition() const;

	std::pair<float, float> PanSpeed() const;
	float RotationSpeed() const;
	float ZoomSpeed() const;
private:
	float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

	glm::mat4 m_ViewMatrix;
	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f }; // re-computed every frame
	glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

	glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

	float m_Distance = 5.0f;
	float m_Pitch = 0.0f, m_Yaw = 0.0f;

	float m_ViewportWidth = 1280, m_ViewportHeight = 720;
};