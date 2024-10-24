#include "Canvas.h"
#include "Viewport.h"

#include <vector>
#include <glm/geometric.hpp>


std::unique_ptr<Program> Brush::_program;
std::unique_ptr<Mesh> Brush::_mesh;
std::unique_ptr<Uniformbuffer> Brush::_brush_ubo;
std::unique_ptr<Uniformbuffer> Brush::_brush_compute_ubo;
std::uint32_t Brush::_brush_ubo_size;

void Brush::Init() {
	_program = Program::Create(TEXT("Brush Program"));
	_program->AddShader("data/brush.vert", GL_VERTEX_SHADER);
	_program->AddShader("data/brush.frag", GL_FRAGMENT_SHADER);
	_program->Compile();

	_mesh = Mesh::Create(TEXT("Brush Mesh"));
	_brush_ubo = Uniformbuffer::Create(TEXT("Brush Display Uniformbuffer"), 2, sizeof(BrushData), nullptr);
	_brush_compute_ubo = Uniformbuffer::Create(TEXT("Brush Compute Uniformbuffer"), 3, sizeof(BrushData) * 64, nullptr);
	_brush_ubo_size = 0;
}

Brush::Brush() {
	_compute_program = Program::Create(TEXT("Brush Compute"));
	_compute_program->AddShader("data/brush.comp", GL_COMPUTE_SHADER);
	_compute_program->Compile();

	GLint work_group_size[3]{};
	glGetProgramiv(_compute_program->ID(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
	_program_work_group_size.x = work_group_size[0];
	_program_work_group_size.y = work_group_size[1];
	_program_work_group_size.z = work_group_size[2];
}

void Brush::SetBrushData(BrushData data) {
	_brush_ubo_size = 1;
	_brush_data = data;
	_brush_ubo->SetData(0, sizeof(BrushData), &_brush_data);
	_brush_compute_ubo->SetData(0, sizeof(BrushData) * _brush_ubo_size, &data);
}

void Brush::SetBrushDatas(std::span<BrushData> data) {
	_brush_ubo_size = std::min((size_t)64, data.size());
	_brush_data = data[_brush_ubo_size - 1];
	_brush_compute_ubo->SetData(0, sizeof(BrushData) * _brush_ubo_size, data.data());
	_brush_ubo->SetData(0, sizeof(BrushData), &data[_brush_ubo_size - 1]);
}

Brush::BrushData Brush::GetBrushData() {
	return _brush_data;
}

void Brush::Paint(Texture* texture) {
	_compute_program->Bind();
	_compute_program->SetUint("brush_datas_count", _brush_ubo_size);
	glBindImageTexture(0, texture->ID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(texture->Width() / _program_work_group_size.x,
					  texture->Height() / _program_work_group_size.y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Brush::PaintLine(Canvas* canvas, BrushData start, BrushData end, float step) {
	auto len = glm::distance(start.position, end.position);
	size_t step_count = std::ceil(len / step);

	std::vector<BrushData> datas;
	datas.reserve(64);

	_brush_data = start;
	float progress = 0.0f;
	for (size_t t = 0; t < step_count; t++) {
		if (t == 0) {
			_brush_data = start;
		} else if (t == step_count - 1) {
			_brush_data = end;
		} else {
			_brush_data.color = glm::mix(start.color, end.color, progress);
			_brush_data.position = glm::mix(start.position, end.position, progress);
			_brush_data.pressure = std::lerp(start.pressure, end.pressure, progress);
			_brush_data.tilt = std::lerp(start.tilt, end.tilt, progress);
			_brush_data.orientation = std::lerp(start.orientation, end.orientation, progress);
			_brush_data.rotation = std::lerp(start.rotation, end.rotation, progress);
		}

		datas.push_back(_brush_data);

		if (datas.size() > 64) {
			SetBrushDatas(datas);
			canvas->Paint(this);
			datas.clear();
		} else if (t == step_count - 1 && !datas.empty()) {
			SetBrushDatas(datas);
			canvas->Paint(this);
			datas.clear();
		}

		progress += 1.0 / (float)step_count;
	}
}

void Brush::Paint(Canvas* canvas, BrushData data) {
	SetBrushData(data);
	canvas->LazyLoad(_brush_data.position, nullptr);
	canvas->Paint(this);
}

void Brush::Render() {
	_program->Bind();
	// _alpha->Bind(1);
	_mesh->Render(GL_TRIANGLES, 6);
	_program->Unbind();
}

void Brush::Refresh() {
	_compute_program->Compile();

	GLint work_group_size[3]{};
	glGetProgramiv(_compute_program->ID(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
	_program_work_group_size.x = work_group_size[0];
	_program_work_group_size.y = work_group_size[1];
	_program_work_group_size.z = work_group_size[2];


	_program->Compile();
}


void Brush::SetColor(glm::vec4 color) { _brush_data.color = color; }
void Brush::SetPosition(glm::vec2 position) { _brush_data.position = position; }
void Brush::SetPressure(float pressure) { _brush_data.pressure = pressure; }
void Brush::SetOrientation(float orientation) { _brush_data.orientation = orientation; }
void Brush::SetTilt(float tilt) { _brush_data.tilt = tilt; }
void Brush::SetRotation(float rotation) { _brush_data.rotation = rotation; }


glm::vec4 Brush::GetColor() { return _brush_data.color; }
glm::vec2 Brush::GetPosition() { return _brush_data.position; }
float Brush::GetPressure() { return _brush_data.pressure; }
float Brush::GetOrientation() { return _brush_data.orientation; }
float Brush::GetTilt() { return _brush_data.tilt; }
float Brush::GetRotation() { return _brush_data.rotation; }