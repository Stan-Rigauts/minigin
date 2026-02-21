#include <algorithm>
#include "Scene.h"
#include <cassert>

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::Remove(const GameObject& object)
{
	auto it = std::find_if(
		m_objects.begin(),
		m_objects.end(),
		[&object](const auto& ptr) { return ptr.get() == &object; });

	if (it != m_objects.end())
		(*it)->MarkForDestroy();
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::Update(float delta_sec)
{
	for (auto& object : m_objects)
	{
		object->Update(delta_sec);
	}
	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[](const auto& obj)
			{
				return obj->IsMarkedForDestroy();
			}),
		m_objects.end());
}

void dae::Scene::FixedUpdate(float Fixed_sec)
{
	for (auto& object : m_objects)
	{
		object->FixedUpdate(Fixed_sec);
	}
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

