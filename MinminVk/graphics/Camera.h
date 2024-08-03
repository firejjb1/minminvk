#pragma once

#include <util/Type.h>
#include <util/Math.h>
#include <Input.h>
#include <graphics/Node.h>

namespace Graphics
{
	struct Camera
	{
	private:
		void computeAngles() {
			// Horizontal direction
			vec3 h_dir = vec3(direction.x, 0.0, -direction.z);
			h_dir = Math::Normalize(h_dir);
			// Compute yaw
			yaw = Math::Degrees(asin(std::abs(h_dir.z)));
			if (h_dir.z >= 0.0) {
				if (h_dir.x >= 0.0) {
					yaw = 360 - yaw;
				}
				else {
					yaw = 180 + yaw;
				}
			}
			else {
				if (h_dir.x >= 0.0) {
					// Nothing
				}
				else {
					yaw = 180 - yaw;
				}
			}
			// Compute pitch
			pitch = Math::Degrees(asin(direction.y));
		}

		vec3 position;
		vec3 up;
		vec3 direction;

		float fov = 45;
		float znear = 0.1f;
		float zfar = 1000.f;
		float width = 1.f;
		float height = 1.f;

		bool mouseLastClicked = false;
		vec2 mouseLastPos;
		float yaw = 0;
		float pitch = 0;


	public:
		SharedPtr<Node> node;

		Camera(NodeManager& nodeManager, vec3 position, vec3 up, vec3 direction, float fov, float znear, float zfar, float width, float height)
			: position{ position }, up{ up }, direction{ direction }, fov{ fov }, znear{ znear }, zfar{ zfar }, width{ width }, height{ height }
		{
			//computeAngles();
			node = nodeManager.AddNode(mat4(1), NodeID{0}, Node::NodeType::CAMERA_NODE);
		}

		mat4 GetCameraMatrix()
		{
			return Math::LookAt(position, position + direction, up);
		}

		mat4 GetProjectionMatrix()
		{
			return Math::Perspective(Math::Radians(fov), width, height, znear, zfar);
		}

		vec3 GetPosition()
		{
			return position;
		}

		void Update(float deltaTime)
		{
			vec3 keyboardMovement(0);
			keyboardMovement.x += Input::D.pressed ? deltaTime : Input::A.pressed ? -deltaTime : 0;
			keyboardMovement.y += Input::W.pressed ? deltaTime : Input::S.pressed ? -deltaTime : 0;
			keyboardMovement.z += Input::Q.pressed ? deltaTime : Input::E.pressed ? -deltaTime : 0;

			vec3 change(0);

			vec3 right = Math::Cross(direction, up);
			change += right * keyboardMovement.x;
			change += direction * keyboardMovement.y;
			change += up * keyboardMovement.z;

			position += change;

			vec2 mousePos = vec2(Input::mouseState.xpos, Input::mouseState.ypos);
			vec2 offset = mousePos - mouseLastPos;
			mouseLastPos = mousePos;

			if (Input::mouseState.leftPressed && mouseLastClicked)
			{
				offset *= deltaTime;
				yaw += offset.x;
				pitch += offset.y;

				// Rotate the view vector by yaw around the vertical axis
				vec3 vAxis{ 0, 1, 0 };
				vec3 View(0,0,-1);
				View = Math::Rotate(View, Math::Radians(-yaw), vAxis);
				View = Math::Normalize(View);

				// Rotate the view vector by the pitch around the horizontal axis
				vec3 hAxis = Math::Cross(View, vAxis);
				hAxis = Math::Normalize(hAxis);
				View = Math::Rotate(View, Math::Radians(-pitch), hAxis);
				View = Math::Normalize(View);

				direction = View;
				direction = Math::Normalize(direction);

				up = Math::Cross(hAxis, direction);
				up = Math::Normalize(up);
			}

			mouseLastClicked = Input::mouseState.leftPressed;


			node->modelMatrix = Math::Inverse(GetCameraMatrix());
			node->isDirty = true;
		}
	};

}