#pragma once

#include "Renderer.h"
#include "AABB.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class Viewport {
public:
	Viewport(glm::ivec2 size);
	~Viewport();

	void SetSize(glm::ivec2 size);
	void SetPosition(glm::vec2 position, bool update = true);
	void SetRotation(float rotation, bool update = true);
	void SetZoom(float zoom, bool update = true);

	glm::ivec2 GetSize() const noexcept;
	glm::vec2 GetPosition() const noexcept;
	float GetRotation() const noexcept;
	float GetZoom() const noexcept;

	bool IsVisible(AABB other);

	void UpdateViewMatrix();
	void UpdateProjMatrix();

	struct Matrices {
		glm::mat4 view;
		glm::mat4 proj;
	} _matrices;

private:
	std::unique_ptr<Uniformbuffer> _buffer;
	
	glm::vec2 _position;
	float _rotation;
	float _zoom;

	glm::ivec2 _size;
	glm::ivec2 _corner;
	AABB _aabb;

};

