/*
 * MeshRenderer.h
 *
 *  Created on: Apr 25, 2017
 *      Author: bog
 */

#ifndef RENDEROPENGL_MESHRENDERER_H_
#define RENDEROPENGL_MESHRENDERER_H_

#include "IRenderable.h"

#include <glm/mat4x4.hpp>
#include <vector>

class Mesh;
class Renderer;

class MeshRenderer: public IRenderable {
public:
	static void init(Renderer* renderer);
	virtual ~MeshRenderer() override;
	static MeshRenderer* get();

	void renderMesh(Mesh& mesh, glm::mat4 worldTransform = glm::mat4(1));

protected:
	MeshRenderer(Renderer* renderer);

private:
	void render(Viewport* pCrtViewport) override;
	void purgeRenderQueue() override;
	void unload() override;

	struct meshRenderData {
		Mesh* pMesh_;
		glm::mat4 wldTransform_;

		meshRenderData(Mesh* pMesh, glm::mat4 wldTransform)
			: pMesh_(pMesh), wldTransform_(wldTransform) {
		}
		meshRenderData(meshRenderData &&) = default;
	};
	std::vector<meshRenderData> renderQueue_;
	unsigned meshShaderProgram_ = 0;
	unsigned indexPos_ = 0;
	unsigned indexNorm_ = 0;
	unsigned indexUV1_ = 0;
	unsigned indexColor_ = 0;
	unsigned indexMatPVW_ = 0;
};

#endif /* RENDEROPENGL_MESHRENDERER_H_ */
