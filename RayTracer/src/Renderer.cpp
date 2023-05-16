#include "Renderer.h"

#include "Walnut/Random.h"
#include <glm/gtc/type_ptr.hpp>

#include <execution>

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage) {
		//No need to resize
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) 
			return;

		m_FinalImage->Resize(width, height);
	}
	else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
	
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
	ResetFrameIndex();

	m_ImageHorizontal.resize(width);
	m_ImageVertical.resize(height);

	for(uint32_t i = 0; i < width; i++)
		m_ImageHorizontal[i] = i;

	for (uint32_t i = 0; i < height; i++)
		m_ImageVertical[i] = i;

}

void Renderer::Render(const Scene& scene, const Camera& camera) {
	// render every pixel
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;
	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#if 1
	std::for_each(std::execution::par, m_ImageVertical.begin(), m_ImageVertical.end(),
		[this](uint32_t y) {
			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {

				glm::vec4 color = PerPixel(x, y);
				m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
			}
		});

#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
		
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()] / (float)m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}

#endif
	m_FinalImage->SetData(m_ImageData);
	if (m_Settings.Accumulate)
	{
		m_FrameIndex++;
	}
	else
	{
		ResetFrameIndex();
	}
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}


		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		float lightIntensity = glm::max(glm::dot(payload.HitNormal, -lightDir), 0.0f); // == cos(angle)

		const Sphere& closestSphere = m_ActiveScene->Spheres[payload.ObjectID];
		const Material& closestSphereMaterial = m_ActiveScene->Materials[closestSphere.MaterialIndex];
		glm::vec3 sphereColor = closestSphereMaterial.Albedo;
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;
		multiplier *= 0.5f;

		ray.origin = payload.HitPosition + payload.HitNormal * 0.00001f;
		ray.direction = glm::reflect(ray.direction, payload.HitNormal + closestSphereMaterial.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	int closestSphere = -1;
	float hitDistance = FLT_MAX;
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++) {
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.origin - sphere.Position;

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

	

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		// Quadratic formula:
		// (-b +- sqrt(discriminant)) / 2a

		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (closestT < hitDistance && closestT >0)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);
	
	return ClosestHit(ray, hitDistance, closestSphere);

	
}


Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectID)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectID = objectID;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectID];

	//float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit distance (currently unused)
	glm::vec3 origin = ray.origin - closestSphere.Position;

	payload.HitPosition = origin + ray.direction * hitDistance;
	payload.HitNormal = glm::normalize(payload.HitPosition);

	payload.HitPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
