#pragma once


#include "core/lux.h"
#include "gui/irenderer.h"


namespace Lux
{
namespace UI
{


	class LUX_GUI_API OpenGLRenderer : public IRenderer
	{
		public:
			OpenGLRenderer() { m_impl = 0; }

			bool create();
			void destroy();

			virtual TextureBase* loadImage(const char* name) LUX_OVERRIDE;
			virtual bool loadFont(const char* path) LUX_OVERRIDE;
			virtual void beginRender(float w, float h) LUX_OVERRIDE;
			virtual void renderImage(TextureBase* image, float* vertices, float* tex_coords, int vertex_count) LUX_OVERRIDE;
			virtual void measureText(const char* text, float* w, float* h) LUX_OVERRIDE;
			virtual void renderText(const char* text, float x, float y, float z) LUX_OVERRIDE;
			virtual void pushScissorArea(float left, float top, float right, float bottom) LUX_OVERRIDE;
			virtual void popScissorArea() LUX_OVERRIDE;
			void setWindowHeight(int height);

		private:
			struct OpenGLRendererImpl* m_impl;
	};


} // ~namespace UI
} // ~namespace Lux
