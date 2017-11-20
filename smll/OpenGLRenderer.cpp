#include <GL/glew.h>
//#include <GL/GL.h>
#include "OpenGLRenderer.hpp"
#include "Model3D.hpp"

#include <iostream>
using namespace std;

//
// Multi-PIE face mark-up
//
// SEE     http://granolaboy.net/temp/dlib-landmark-mean.png
//
// landmark indices used for mask placement
//
#define FACE_CENTER         (28)
#define FACE_LEFT           (36)
#define FACE_RIGHT          (45)
#define EYE_LEFT            (36)
#define EYE_RIGHT           (45)



namespace smll {


	OpenGLRenderer::OpenGLRenderer()
		: m_r(255)
		, m_g(255)
		, m_b(255)
		, m_a(255)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBindTexture(GL_TEXTURE_2D, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		m_viewportWidth = viewport[2];
		m_viewportHeight = viewport[3];
	}

	OpenGLRenderer::~OpenGLRenderer()
	{

	}


	void OpenGLRenderer::SetDrawColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
	{
		glColor4ub(red, green, blue, alpha);
		m_r = red;
		m_g = green;
		m_b = blue;
		m_a = alpha;
	}

	void OpenGLRenderer::SetLineWidth(float w)
	{
		glLineWidth(w);
	}

	void OpenGLRenderer::SetViewport(int w, int h)
	{
		m_viewportWidth = w;
		m_viewportHeight = h;
	}

	int OpenGLRenderer::LoadTexture(const ImageWrapper& image)
	{
		GLuint t;
		glGenTextures(1, &t);

		ReLoadTexture(image, t);

		return t;
	}

	void OpenGLRenderer::ReLoadTexture(const ImageWrapper& image, int texture)
	{
		glEnable(GL_TEXTURE_2D);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, image.getStride() / image.getNumElems());
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, image.w, image.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRenderer::DrawBegin()
	{
		glEnable(GL_TEXTURE_2D);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::DrawEnd()
	{
		// up to app to flip gl buffers
	}


	void OpenGLRenderer::DrawImage(const ImageWrapper& frame)
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.getStride() / frame.getNumElems());

		// load the camera texture
		if (frame.type == IMAGETYPE_BGR)
			glTexImage2D(GL_TEXTURE_2D, 0, 3, frame.w, frame.h, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
		else if (frame.type == IMAGETYPE_BGRA)
			glTexImage2D(GL_TEXTURE_2D, 0, 4, frame.w, frame.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame.data);

		// draw full frame
		drawBox();
	}

	static float fova = 40;

	void OpenGLRenderer::DrawFaces(const Faces& faces, int numFaces)
	{
		for (int i = 0; i < numFaces; i++)
		{
			glColor3ub(0, 0, 255);
			DrawRect(faces[i].GetBounds());
			glColor3ub(0, 255, 0);
			DrawLandmarks(faces[i].GetPoints());

			glMatrixMode(GL_PROJECTION);
			float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
			glLoadIdentity();
			gluPerspective(fova, aspect, 1, 20000);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			makeTransform(faces[i]);
			glLoadMatrixd(m_transform);

			// draw the axis (debug)
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
			glColor4ub(255, 0, 0, 255);
			glVertex3f(0, 0, 0);
			glVertex3f(1000, 0, 0);
			glColor4ub(0, 255, 0, 255);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 1000, 0);
			glColor4ub(0, 0, 255, 255);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 1000);
			glEnd();

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}

		glColor4ub(m_r, m_g, m_b, m_a);
	}

	void OpenGLRenderer::DrawLandmarks(const dlib::point* points)
	{
		// landmarks
		drawLine(points, 0, 16);           // Jaw line
		drawLine(points, 17, 21);          // Left eyebrow
		drawLine(points, 22, 26);          // Right eyebrow
		drawLine(points, 27, 30);          // Nose bridge
		drawLine(points, 30, 35, true);    // Lower nose
		drawLine(points, 36, 41, true);    // Left eye
		drawLine(points, 42, 47, true);    // Right Eye
		drawLine(points, 48, 59, true);    // Outer lip
		drawLine(points, 60, 67, true);    // Inner lip

		/*
				// reverse the jaw line to guess top of head
				long cx = (points[0].x + points[16].x) / 2;
				long cy = (points[0].y + points[16].y) / 2;
				for (int i = 0; i <= 16; i++)
				{
					long x = cx - (points[i].x - cx);
					long y = cy - (points[i].y - cy);
					points.push_back(cv::Point(x, y));
				}
		*/
	}

	void OpenGLRenderer::DrawRect(const dlib::rectangle& r)
	{
		// rect
		dlib::point p[4];
		p[0] = dlib::point(r.left(), r.top());
		p[1] = dlib::point(r.right(), r.top());
		p[2] = dlib::point(r.right(), r.bottom());
		p[3] = dlib::point(r.left(), r.bottom());
		drawLine(p, 0, 3, true);
	}

	void   OpenGLRenderer::DrawGlasses(const Face& face, int texture)
	{
		glMatrixMode(GL_PROJECTION);
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		glLoadIdentity();
		gluPerspective(fova, aspect, 1, 20000);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		makeTransform(face);
		glLoadMatrixd(m_transform);

		// load the glasses
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);

		//model_points.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // Left eye left corner
		//model_points.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // Right eye right corner
		float x = 300.0f;
		float y = 150.0f;
		float z = -150.0f;
		float dy = 150.0f;
		float dz = 2 * x;

		// draw
		glBegin(GL_QUADS);

		// front
		glTexCoord2f(0, 1);
		glVertex3f(x, y + dy, z);
		glTexCoord2f(1, 1);
		glVertex3f(-x, y + dy, z);
		glTexCoord2f(1, 0.5);
		glVertex3f(-x, y - dy, z);
		glTexCoord2f(0, 0.5);
		glVertex3f(x, y - dy, z);

		// side
		glTexCoord2f(1, 0);
		glVertex3f(x, y - dy, z);
		glTexCoord2f(0, 0);
		glVertex3f(x, y - dy, z - dz);
		glTexCoord2f(0, 0.5);
		glVertex3f(x, y + dy, z - dz);
		glTexCoord2f(1, 0.5);
		glVertex3f(x, y + dy, z);

		// side
		glTexCoord2f(1, 0.5);
		glVertex3f(-x, y + dy, z);
		glTexCoord2f(0, 0.5);
		glVertex3f(-x, y + dy, z - dz);
		glTexCoord2f(0, 0);
		glVertex3f(-x, y - dy, z - dz);
		glTexCoord2f(1, 0);
		glVertex3f(-x, y - dy, z);

		glEnd();


		// unbind
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}


	void   OpenGLRenderer::DrawMask(const Face& face, int texture)
	{
		glMatrixMode(GL_PROJECTION);
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		glLoadIdentity();
		gluPerspective(fova, aspect, 1, 20000);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		makeTransform(face);
		glLoadMatrixd(m_transform);

		// load the mask
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);

		//model_points.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // Left eye left corner
		//model_points.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // Right eye right corner
		float x = 500.0f;
		float y = 100.0f;
		float z = -150.0f;
		float dy = x;

		// draw
		glBegin(GL_QUADS);

		// front
		glTexCoord2f(0, 1);
		glVertex3f(x, y + dy, z);
		glTexCoord2f(1, 1);
		glVertex3f(-x, y + dy, z);
		glTexCoord2f(1, 0);
		glVertex3f(-x, y - dy, z);
		glTexCoord2f(0, 0);
		glVertex3f(x, y - dy, z);

		glEnd();


		// unbind
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}



	void   OpenGLRenderer::DrawModel(const Face& face, const Model3D& model, int texture)
	{
		glMatrixMode(GL_PROJECTION);
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		glLoadIdentity();
		gluPerspective(fova, aspect, 1, 20000);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		makeTransform(face);
		glLoadMatrixd(m_transform);

		// load the mask
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, texture);

		// draw
		glBegin(GL_QUADS);

		for (int i = 0; i < model.faces.size(); i++)
		{
			for (int j = 0; j < 4; j++)
			{
				const Face3D& f3d = model.faces[i];
				glTexCoord2f(model.textureCoords[f3d.textureCoordIndices[j]].x(),
					model.textureCoords[f3d.textureCoordIndices[j]].y());
				//glNormal3f(model.normals[f3d.normalIndices[j]].x(),
				//	model.normals[f3d.normalIndices[j]].y(),
				//	model.normals[f3d.normalIndices[j]].z());
				glVertex3f(model.vertices[f3d.vertexIndices[j]].x(),
					model.vertices[f3d.vertexIndices[j]].y(),
					model.vertices[f3d.vertexIndices[j]].z());
			}
		}

		glEnd();

		// unbind
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_CULL_FACE);
	}











	// ---------------------------------------------------------------------------------------------
	// drawBox : draw a quad (-1,-1) to (1,1)
	//
	void  OpenGLRenderer::drawBox(void)
	{
		glBegin(GL_QUADS);

		glTexCoord2f(0, 0);
		glVertex3f(-1, -1, 0);

		glTexCoord2f(1, 0);
		glVertex3f(1, -1, 0);

		glTexCoord2f(1, 1);
		glVertex3f(1, 1, 0);

		glTexCoord2f(0, 1);
		glVertex3f(-1, 1, 0);

		glEnd();
	}

	void	OpenGLRenderer::drawLine(const dlib::point* points, int start, int end, bool closed)
	{
		// Set up reverse viewport transform
		glPushMatrix();
		glTranslatef(-1, -1, 0);
		glScalef(2.0f / (float)m_viewportWidth, 2.0f / (float)m_viewportHeight, 1.0f);

		// draw lines
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_STRIP);

		// verts
		for (int i = start; i <= end; i++)
		{
			float x = points[i].x();
			float y = m_viewportHeight - points[i].y() - 1;
			glVertex3f(x, y, 0);
		}
		// repeat first vert if closed
		if (closed)
		{
			float x = points[start].x();
			float y = m_viewportHeight - points[start].y() - 1;
			glVertex3f(x, y, 0);
		}

		// done
		glEnd();
		glEnable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	void OpenGLRenderer::makeTransform(const Face& face)
	{
		const cv::Mat& rotationVector = face.RotationVector();
		const cv::Mat& translationVector = face.TranslationVector();

		// From:
		// http://answers.opencv.org/question/23089/opencv-opengl-proper-camera-pose-using-solvepnp/
		//
		// The 3 most important things to know are that :
		//
		//	1) solvePnP gives you the transfer matrix from the model's frame (ie the cube) to the 
		//     camera's frame (it's called view matrix).
		//  2) The camera's frame are not the same in opencv and opengl. Y axis and Z axis are inverted.
		//	3) How matrixes are stored is not the same neither.Opengl matrixes are column major order
		//     whereas they are row major order in Opencv.
		//

		// Build the view matrix
		//
		cv::Mat rotation = cv::Mat::zeros(3, 3, CV_64F);
		cv::Mat viewMatrix = cv::Mat::zeros(4, 4, CV_64F);

		// convert rotation vector to rotation matrix
		// - Note: in openCV a rotation vector is a 1x3 or 3x1 matrix with the euler rotation
		//         values for rotation around the x,y,z axis are stored in the respective locations
		//
		cv::Rodrigues(rotationVector, rotation);

		// copy 3x3 rotation matrix and translation vector into view matrix
		for (unsigned int row = 0; row<3; ++row)
		{
			// rotation
			for (unsigned int col = 0; col<3; ++col)
			{
				viewMatrix.at<double>(row, col) = rotation.at<double>(row, col);
			}
			// translation
			viewMatrix.at<double>(row, 3) = translationVector.at<double>(row, 0);
		}
		viewMatrix.at<double>(3, 3) = 1.0f;

		// Convert openCV --> openGL
		//           LEFT     RIGHT
		//
		cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
		cvToGl.at<double>(0, 0) = 1.0f;
		cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis for openGL, not for OBS...this sucks right here
		cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis 
		cvToGl.at<double>(3, 3) = 1.0f;
		viewMatrix = cvToGl * viewMatrix;

		// Transpose and save 
		//
		cv::Mat glViewMatrix = cv::Mat::zeros(4, 4, CV_64F);
		cv::transpose(viewMatrix, glViewMatrix);
		memcpy(m_transform, &glViewMatrix.at<double>(0, 0), sizeof(double) * 16);
	}


} // smll namespace

