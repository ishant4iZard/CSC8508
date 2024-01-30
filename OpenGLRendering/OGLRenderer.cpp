/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLMesh.h"
#include "OGLTexture.h"

#include "SimpleFont.h"
#include "TextureLoader.h"

#include "Mesh.h"

#ifdef _WIN32
#include "Win32Window.h"

#include "KHR\khrplatform.h"
#include "glad\gl.h"
#include "KHR/WGLext.h"

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
#endif

using namespace NCL;
using namespace NCL::Rendering;

using std::string;

#ifdef OPENGL_DEBUGGING
static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif;

OGLRenderer::OGLRenderer(Window& w) : RendererBase(w)	{
	initState = false;
#ifdef _WIN32
	InitWithWin32(w);
#endif
	boundMesh		= nullptr;
	activeShader	= nullptr;

	windowSize = w.GetScreenSize();
}

OGLRenderer::~OGLRenderer()	{
#ifdef _WIN32
	DestroyWithWin32();
#endif
}

void OGLRenderer::OnWindowResize(int w, int h)	 {
	windowSize = Vector2i(w, h);

	glViewport(0, 0, windowSize.x, windowSize.y);
}

void OGLRenderer::BeginFrame()		{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(0);
	glUseProgram(0);
}

void OGLRenderer::RenderFrame()		{

}

void OGLRenderer::EndFrame()		{
}

void OGLRenderer::SwapBuffers()   {
	::SwapBuffers(deviceContext);
}

void OGLRenderer::BindShader(const OGLShader& oglShader) {
	if (!oglShader.LoadSuccess()) {
		std::cout << __FUNCTION__ << ": shader has failed to load correctly!\n";
		activeShader = nullptr;
		return;
	}
	glUseProgram(oglShader.GetProgramID());
	activeShader = &oglShader;
}

void OGLRenderer::BindMesh(const OGLMesh& m) {
	if (m.GetVAO() == 0) {
		std::cout << __FUNCTION__ << ": Mesh has not been uploaded!\n";
	}
	glBindVertexArray(m.GetVAO());
	boundMesh = &m;
}

void OGLRenderer::DrawBoundMesh(uint32_t subLayer, uint32_t numInstances) {
	if (!boundMesh) {
		std::cout << __FUNCTION__ << " has been called without a bound mesh!\n";
		return;
	}
	if (!activeShader) {
		std::cout << __FUNCTION__ << " has been called without a bound shader!\n";
		return;
	}
	GLuint	mode	= 0;
	int		count	= 0;
	int		offset	= 0;

	if (boundMesh->GetSubMeshCount() == 0) {
		if (boundMesh->GetIndexCount() > 0) {
			count = boundMesh->GetIndexCount();
		}
		else{
			count = boundMesh->GetVertexCount();
		}
	}
	else {
		const SubMesh* m = boundMesh->GetSubMesh(subLayer);
		offset = m->start;
		count  = m->count;
	}

	switch (boundMesh->GetPrimitiveType()) {
		case GeometryPrimitive::Triangles:		mode = GL_TRIANGLES;		break;
		case GeometryPrimitive::Points:			mode = GL_POINTS;			break;
		case GeometryPrimitive::Lines:			mode = GL_LINES;			break;
		case GeometryPrimitive::TriangleFan:	mode = GL_TRIANGLE_FAN;		break;
		case GeometryPrimitive::TriangleStrip:	mode = GL_TRIANGLE_STRIP;	break;
		case GeometryPrimitive::Patches:		mode = GL_PATCHES;			break;
	}

	if (numInstances > 1) {
		if (boundMesh->GetIndexCount() > 0) {
			glDrawElementsInstanced(mode, count, GL_UNSIGNED_INT, (const GLvoid*)(offset * sizeof(unsigned int)), numInstances);
		}
		else {
			glDrawArraysInstanced(mode, 0, count, numInstances);
		}
	}
	else {
		if (boundMesh->GetIndexCount() > 0) {
			glDrawElements(mode, count, GL_UNSIGNED_INT, (const GLvoid*)(offset * sizeof(unsigned int)));
		}
		else {
			glDrawArrays(mode, 0, count);
		}
	}
}

void OGLRenderer::BindTextureToShader(const OGLTexture& t, const std::string& uniform, int texUnit) const{
	GLint texID = t.GetObjectID();

	if (!activeShader) {
		std::cout << __FUNCTION__ << " has been called without a bound shader!\n";
		return;//Debug message time!
	}
	
	GLuint slot = glGetUniformLocation(activeShader->GetProgramID(), uniform.c_str());

	if (slot < 0) {

		return;
	}

	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texID);

	glUniform1i(slot, texUnit);
}

#ifdef _WIN32
void OGLRenderer::InitWithWin32(Window& w) {
	Win32Code::Win32Window* realWindow = (Win32Code::Win32Window*)&w;

	if (!(deviceContext = GetDC(realWindow->GetHandle()))) {
		std::cout << __FUNCTION__ << " Failed to create window!\n";
		return;
	}

	//A pixel format descriptor is a struct that tells the Windows OS what type of front / back buffers we want to create etc
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize			= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion		= 1;
	pfd.dwFlags			= PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;   //It must be double buffered, it must support OGL(!), and it must allow us to draw to it...
	pfd.iPixelType		= PFD_TYPE_RGBA;	//We want our front / back buffer to have 4 channels!
	pfd.cColorBits		= 32;				//4 channels of 8 bits each!
	pfd.cDepthBits		= 24;				//24 bit depth buffer
	pfd.cStencilBits	= 8;				//plus an 8 bit stencil buffer
	pfd.iLayerType		= PFD_MAIN_PLANE;

	GLuint		PixelFormat;
	if (!(PixelFormat = ChoosePixelFormat(deviceContext, &pfd))) {	// Did Windows Find A Matching Pixel Format for our PFD?
		std::cout << __FUNCTION__ << " Failed to choose a pixel format!\n";
		return;
	}

	if (!SetPixelFormat(deviceContext, PixelFormat, &pfd)) {		// Are We Able To Set The Pixel Format?
		std::cout << __FUNCTION__ << " Failed to set a pixel format!\n";
		return;
	}

	HGLRC		tempContext;		//We need a temporary OpenGL context to check for OpenGL 3.2 compatibility...stupid!!!
	if (!(tempContext = wglCreateContext(deviceContext))) {	// Are We Able To get the temporary context?
		std::cout << __FUNCTION__ <<"  Cannot create a temporary context!\n";
		wglDeleteContext(tempContext);
		return;
	}

	if (!wglMakeCurrent(deviceContext, tempContext)) {	// Try To Activate The Rendering Context
		std::cout << __FUNCTION__ << " Cannot set temporary context!\n";
		wglDeleteContext(tempContext);
		return;
	}
	if (!gladLoaderLoadGL()) {
		std::cout << __FUNCTION__ << " Cannot initialise GLAD!\n";	//It's all gone wrong!
		return;
	}
	//Now we have a temporary context, we can find out if we support OGL 4.x
	char* ver = (char*)glGetString(GL_VERSION); // ver must equal "4.1.0" (or greater!)
	int major = ver[0] - '0';		//casts the 'correct' major version integer from our version string
	int minor = ver[2] - '0';		//casts the 'correct' minor version integer from our version string

	if (major < 4) {					//Graphics hardware does not support OGL 4! Erk...
		std::cout << __FUNCTION__ << " Device does not support OpenGL 4.x!\n";
		wglDeleteContext(tempContext);
		return;
	}

	if (major == 4 && minor < 1) {	//Graphics hardware does not support ENOUGH of OGL 4! Erk...
		std::cout << __FUNCTION__ << " Device does not support at least OpenGL 4.1!\n";
		wglDeleteContext(tempContext);
		return;
	}
	//We do support OGL 4! Let's set it up...

	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#ifdef OPENGL_DEBUGGING 
		| WGL_CONTEXT_DEBUG_BIT_ARB
#endif		//No deprecated stuff!! DIE DIE DIE glBegin!!!!
		,WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0					//That's enough attributes...
	};

	//Everywhere else in the Renderers, we use function pointers provided by GLEW...but we can't initialise GLEW yet! So we have to use the 'Wiggle' API
	//to get a pointer to the function that will create our OpenGL 3.2 context...
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	renderContext = wglCreateContextAttribsARB(deviceContext, 0, attribs);

	// Check for the context, and try to make it the current rendering context
	if (!renderContext || !wglMakeCurrent(deviceContext, renderContext)) {
		std::cout << __FUNCTION__ <<" Cannot set OpenGL 3 context!" << std::endl;	//It's all gone wrong!
		wglDeleteContext(renderContext);
		wglDeleteContext(tempContext);
		return;
	}

	wglDeleteContext(tempContext);	//We don't need the temporary context any more!

	std::cout << __FUNCTION__ << " Initialised OpenGL " << major << "." << minor << " rendering context" << std::endl;	//It's all gone wrong!

	glEnable(GL_FRAMEBUFFER_SRGB);

#ifdef OPENGL_DEBUGGING
	glDebugMessageCallback(DebugCallback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	//If we get this far, everything's going well!
	initState = true; 

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	w.SetRenderer(this);
}

void OGLRenderer::DestroyWithWin32() {
	wglDeleteContext(renderContext);
}

bool OGLRenderer::SetVerticalSync(VerticalSyncState s) {
	if (!wglSwapIntervalEXT) {
		return false;
	}
	GLuint state;

	switch (s) {
		case VerticalSyncState::VSync_OFF:		state =  0; break;
		case VerticalSyncState::VSync_ON:		state =  1; break;
		case VerticalSyncState::VSync_ADAPTIVE:	state = -1; break;
	}

	return wglSwapIntervalEXT(state);
}
#endif

#ifdef OPENGL_DEBUGGING
static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	string sourceName;
	string typeName;
	string severityName;

	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceName = "Source(OpenGL)"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceName = "Source(Window System)"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceName = "Source(Shader Compiler)"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceName = "Source(Third Party)"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceName = "Source(Application)"; break;
	case GL_DEBUG_SOURCE_OTHER: sourceName = "Source(Other)"; break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: typeName = "Type(Error)"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeName = "Type(Deprecated Behaviour)"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeName = "Type(Undefined Behaviour)"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeName = "Type(Portability)"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeName = "Type(Performance)"; break;
	case GL_DEBUG_TYPE_OTHER: typeName = "Type(Other)"; break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: severityName = "Priority(High)"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityName = "Priority(Medium)"; break;
	case GL_DEBUG_SEVERITY_LOW: severityName = "Priority(Low)"; break;
	}

	std::cout << "OpenGL Debug Output: " + sourceName + ", " + typeName + ", " + severityName + ", " + string(message) + "\n";
}
#endif