#include "Node.h"
#include <util/Math.h>

namespace Graphics
{
	void Node::Update(f32 deltaTime, NodeManager& nodeManager)
	{
		timer += deltaTime;
		if (timer > maxAnimationTime)
			timer = minAnimationTime;
		mat4 newrot(1);
		mat4 newscale(1);
		mat4 newtrans(1);
		for (auto& anim : animations)
		{
			if (anim->animationType == Animation::AnimationType::ROTATION)
			{
				vec4 rot = anim->Sample(timer);
				quat quatRot(rot.x, rot.y, rot.z, rot.w);
				newrot = Math::RotateQuat(quatRot);

				isDirty = true;
			}
			else if (anim->animationType == Animation::AnimationType::SCALE)
			{
				vec3 scale = anim->Sample(timer);
				newscale = Math::Scale(mat4(1), scale);
				isDirty = true;
			}
			else if (anim->animationType == Animation::AnimationType::TRANSLATION)
			{
				vec3 trans = anim->Sample(timer);
				newtrans = Math::Translate(mat4(1), trans);
				isDirty = true;
			}
			else if (anim->animationType == Animation::AnimationType::WEIGHTS)
			{
				vec4 weights = anim->Sample(timer);
				
				for (int i = 0; i < Min((u32)4, anim->numWeightsMorphTarget); ++i)
				{
					morphWeights[i] = weights[i];
				}
			}
		}
		mat4 newModel = newtrans * newrot * newscale;
		if (animations.size() == 0)
			newModel = modelMatrix;
		if (isDirty)
		{
			worldMatrix = parentModelMatrix * newModel;

			for (NodeID childID : childrenIDs)
			{
				auto childNode = nodeManager.GetNode(childID);
				childNode->isDirty = true;
				childNode->parentModelMatrix = worldMatrix;
			}
			isDirty = false;
		}
	}
}