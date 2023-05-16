#pragma once
#include <memory>

#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include "glm/glm.hpp"



class Renderer {

public:
	struct Settings {
		bool Accumulate = true;
	};
public:
	Renderer() = default;

	void Render(const Scene& scene, const Camera& camera);

	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
private:
	struct HitPayload {
		float HitDistance;
		glm::vec3 HitPosition;
		glm::vec3 HitNormal;

		int ObjectID;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); //Ray Generation - Vulkan Naming

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectID);
	HitPayload Miss(const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	float m_AspectRatio = 0.0f;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontal, m_ImageVertical;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_FrameIndex = 1;
};
