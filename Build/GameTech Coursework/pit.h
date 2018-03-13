#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>

using namespace CommonUtils;

//Fully striped back scene to use as a template for new scenes.
class pit : public Scene
{
public:
	pit(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~pit()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();

		//Who doesn't love finding some common ground?
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.5f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"pitWall1",
			Vector3(1.0f, 0.5f, 1.5f),
			Vector3(5.0f, 1.5f, 1.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"pitWall2",
			Vector3(1.0f, 0.5f, -5.5f),
			Vector3(5.0f, 1.5f, 1.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"pitWall3",
			Vector3(-5.5f, 0.5f, -3.0f),
			Vector3(1.0f, 1.5f, 5.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"pitWall3",
			Vector3(5.0f, 0.5f, -3.5f),
			Vector3(-1.0f, 1.5f, -5.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		auto create_ball_cube = [&](const Vector3& offset, const Vector3& scale, float ballsize)
		{
			const int dims = 10;
			const Vector4 col = Vector4(1.0f, 0.5f, 0.2f, 1.0f);

			for (int x = 0; x < dims; ++x)
			{
				for (int y = 0; y < dims; ++y)
				{
					for (int z = 0; z < dims; ++z)
					{
						Vector3 pos = offset + Vector3(scale.x *x, scale.y * y, scale.z * z);
						GameObject* sphere = BuildSphereObject(
							"",					// Optional: Name
							pos,				// Position
							ballsize,			// Half-Dimensions
							true,				// Physics Enabled?
							10.f,				// Physical Mass (must have physics enabled)
							true,				// Physically Collidable (has collision shape)
							true,				// Dragable by user?
							col);// Render color
						this->AddGameObject(sphere);
					}
				}
			}
		};

		create_ball_cube(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f), 0.1f);


	}

};