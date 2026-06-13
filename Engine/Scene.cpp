#include "Scene.h"
#include "Scene.h"
#include "Scene.h"
#include <algorithm>
#include "Scene.h"
#include <cassert>
#include "RenderComponent.h"
using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_pendingObjects.emplace_back(std::move(object)); 
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

	assert(this != nullptr && "Scene pointer is null!");
	assert(m_objects.size() < 1000 && "m_objects size is corrupt!");

	for (auto& obj : m_pendingObjects)
		m_objects.emplace_back(std::move(obj));
	m_pendingObjects.clear();

	const int count = (int)m_objects.size();
	for (int i = 0; i < count; ++i)
	{
		if (!m_objects[i]->IsMarkedForDestroy())
			m_objects[i]->Update(delta_sec);
	}

	m_objects.erase(
		std::remove_if(m_objects.begin(), m_objects.end(),
			[](const auto& obj) { return obj->IsMarkedForDestroy(); }),
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
	std::vector<GameObject*> renderList;
	renderList.reserve(m_objects.size());

	for (const auto& obj : m_objects)
		renderList.push_back(obj.get());

	std::sort(renderList.begin(), renderList.end(),
		[](GameObject* objectA, GameObject* objectB)
		{
			auto* renderComponentA = objectA->GetComponent<RenderComponent>();
			auto* renderComponentB = objectB->GetComponent<RenderComponent>();

			int layerA = renderComponentA ? renderComponentA->GetLayer() : 0;
			int layerB = renderComponentB ? renderComponentB->GetLayer() : 0;

			if (layerA == layerB)
				return objectA < objectB;

			return layerA < layerB;
		});

	for (const auto& object : renderList)
		object->Render();
}
const std::vector<std::unique_ptr<GameObject>>& dae::Scene::GetObjects() const
{
	return m_objects;
}



void dae::Scene::RemoveObjectsWithTag(const char* tag)
{
	for (auto& go : m_objects)
	{
		if (go->GetTag() == tag)
			go->MarkForDestroy();
	}
	m_pendingObjects.erase(
		std::remove_if(m_pendingObjects.begin(), m_pendingObjects.end(),
			[tag](const std::unique_ptr<GameObject>& go) {
				return go->GetTag() == tag;
			}),
		m_pendingObjects.end());
}