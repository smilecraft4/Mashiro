#pragma once
#include "Renderer.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Canvas;
class Viewport;

/*
class ITool {
public:
	virtual void Use() = 0;
};
*/

class Brush {
public:
	static void Init();

	// TODO: use stylus input packet directly
	struct BrushData {
		// TODO: Convert this data to canvas coord
		float pressure;
		float tilt;
		float orientation;
		float rotation;
		glm::vec2 position;		// Window relative pos
		float padding[2];
		glm::vec4 color;		// Color + Opacity
		// this is temporay in the future this should use other type of data namely the parameter of the brush (hardness, radius, etc...)
	};

	Brush(const Brush&) = delete;
	Brush(Brush&&) = delete;
	Brush& operator=(const Brush&) = delete;
	Brush& operator=(Brush&&) = delete;

	Brush();

	void SetColor(glm::vec4 color);
	void SetPosition(glm::vec2 position);
	void SetPressure(float pressure);
	void SetOrientation(float orientation);
	void SetTilt(float tilt);
	void SetRotation(float rotation);

	glm::vec4 GetColor();
	glm::vec2 GetPosition();
	float GetPressure();
	float GetOrientation();
	float GetTilt();
	float GetRotation();

	void SetBrushData(BrushData data);
	void SetBrushDatas(std::span<BrushData> data);
	BrushData GetBrushData();

	void Paint(Texture* texture);
	void PaintLine(Canvas* canvas, BrushData start, BrushData end, float step);
	void Paint(Canvas* canvas, BrushData data);
	void Render();
	void Refresh();

private:
	BrushData _brush_data;

	std::unique_ptr<Texture> _alpha;
	std::unique_ptr<Program> _compute_program;
	glm::ivec3 _program_work_group_size;

	static std::unique_ptr<Program> _program;
	static std::unique_ptr<Mesh> _mesh;
	static std::unique_ptr<Uniformbuffer> _brush_ubo;
	static std::unique_ptr<Uniformbuffer> _brush_compute_ubo;
	static std::uint32_t _brush_ubo_size;
};

