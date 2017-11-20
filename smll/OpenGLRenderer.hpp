#ifndef __SMLL_OPENGL_RENDERER_HPP__
#define __SMLL_OPENGL_RENDERER_HPP__

#include "FaceDetector.hpp"
#include "Model3D.hpp"

namespace smll {

	class OpenGLRenderer
	{
	public:

		OpenGLRenderer();
		~OpenGLRenderer();

		void	SetDrawColor(unsigned char red, 
									 unsigned char green, 
									 unsigned char blue, 
									 unsigned char alpha = 255);
		void	SetLineWidth(float w);
		void	SetViewport(int w, int h);

		int		LoadTexture(const ImageWrapper& image);
		void	ReLoadTexture(const ImageWrapper& image, int texture);

		void	DrawBegin();
		void	DrawEnd();
		void	DrawImage(const ImageWrapper& frame);
		void	DrawFaces(const Faces& faces, int numFaces);
		void	DrawLandmarks(const dlib::point* points);
		void	DrawRect(const dlib::rectangle& r);
		void    DrawGlasses(const Face& face, int texture);
		void    DrawMask(const Face& face, int texture);
		void    DrawModel(const Face& face, const Model3D& model, int texture);

	private:

		// gl texture names
		std::vector<GLuint>			m_textures;

		// draw color
		GLubyte						m_r, m_g, m_b, m_a;

		// transform
		GLdouble					m_transform[16];

		// viewport dimensions
		int							m_viewportWidth;
		int							m_viewportHeight;

		void						drawBox(void);
		void						drawLine(const dlib::point* points, int start, int end, bool closed=false);
		void						makeTransform(const Face& face);
	};



} // smll namespace
#endif // __SMLL_OPENGL_RENDERER_HPP__

