#include "GL1Frontend.h"
#include "ui/UI.h"
#include "client/ClientConsole.h"
#include "textures/TextureCoords.h"
#include "common/Log.h"
#include "common/DateUtil.h"
#include "common/FileSystem.h"
#include "common/ConfigManager.h"
#include <SDL_image.h>

#if SDL_VIDEO_OPENGL
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
//#include <SDL_opengl_glext.h>
#include "GLShared.h"
#else
typedef unsigned int TexNum;
#endif

struct TextureData {
	int unused;
};

// TODO: use for batching
static float vertices[2048];

inline TexNum GL1getTexNum (TextureData *textureData)
{
	const intptr_t texnum = reinterpret_cast<intptr_t>(textureData);
	return texnum;
}

GL1Frontend::GL1Frontend (std::shared_ptr<IConsole> console) :
		SDLFrontend(console), _maxTextureUnits(0), _currentTextureUnit(nullptr), _rx(1.0f), _ry(1.0f)
{
	_context = nullptr;

	memset(&_viewPort, 0, sizeof(_viewPort));
	memset(_colorArray, 0, sizeof(_colorArray));
}

GL1Frontend::~GL1Frontend ()
{
	if (_context)
		SDL_GL_DeleteContext(_context);
}

inline bool GL1Frontend::invalidTexUnit (int textureUnit) const
{
	return textureUnit < 0 || textureUnit >= _maxTextureUnits || textureUnit >= MAX_GL_TEXUNITS;
}

void GL1Frontend::setGLAttributes ()
{
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

void GL1Frontend::setHints ()
{
}

void GL1Frontend::enableScissor (int x, int y, int width, int height)
{
#ifdef SDL_VIDEO_OPENGL
	const int lowerLeft = std::max(0, getHeight() - y - height);
	glScissor(x * _rx, lowerLeft * _ry, width * _rx, height * _ry);
	glEnable(GL_SCISSOR_TEST);
#endif
}

float GL1Frontend::getWidthScale () const
{
	return _rx;
}

float GL1Frontend::getHeightScale () const
{
	return _ry;
}

void GL1Frontend::disableScissor ()
{
#ifdef SDL_VIDEO_OPENGL
	glDisable(GL_SCISSOR_TEST);
#endif
}

void GL1Frontend::initRenderer ()
{
#ifdef SDL_VIDEO_OPENGL
	Log::info(LOG_CLIENT, "init opengl renderer");

	_context = SDL_GL_CreateContext(_window);

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &_maxTextureUnits);
	Log::info(LOG_CLIENT, "max texture units: %i", _maxTextureUnits);
	GL_checkError();

	_maxTextureUnits = clamp(_maxTextureUnits, 1, MAX_GL_TEXUNITS);

	for (int i = 0; i < _maxTextureUnits; ++i) {
		_texUnits[i].textureUnit = GL_TEXTURE0 + i;
		enableTextureUnit(_texUnits[i], true);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		enableTextureUnit(_texUnits[i], false);
		GL_checkError();
	}
	enableTextureUnit(_texUnits[0], true);

	glClearColor(0, 0, 0, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	GL_checkError();
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	GL_checkError();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_checkError();
	glAlphaFunc(GL_GREATER, 0.1f);
	GL_checkError();
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	GL_checkError();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertices);

	glEnableClientState(GL_COLOR_ARRAY);

	GL_checkError();
#endif
}

int GL1Frontend::getCoordinateOffsetX () const
{
	return -_viewPort.x;
}

int GL1Frontend::getCoordinateOffsetY () const
{
	return -_viewPort.y;
}

void GL1Frontend::getViewPort (int* x, int *y, int *w, int *h) const
{
	if (x != nullptr)
		*x = _viewPort.x;
	if (y != nullptr)
		*y = _viewPort.y;
	if (w != nullptr)
		*w = _viewPort.w;
	if (h != nullptr)
		*h = _viewPort.h;
}

void GL1Frontend::enableTextureUnit (TexUnit &texunit, bool enable)
{
	if (enable == texunit.active) {
		return;
	}

	texunit.active = enable;
	_currentTextureUnit = &texunit;
#ifdef SDL_VIDEO_OPENGL
	if (enable) {
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif
}

void GL1Frontend::bindTexture (Texture* texture, int textureUnit)
{
	if (invalidTexUnit(textureUnit))
		return;

	const TexNum texnum = GL1getTexNum(texture->getData());
	if (_currentTextureUnit->currentTexture == texnum)
		return;
	_currentTextureUnit->currentTexture = texnum;
#ifdef SDL_VIDEO_OPENGL
	glBindTexture(GL_TEXTURE_2D, _currentTextureUnit->currentTexture);
#endif
}

void GL1Frontend::setColorPointer (const Color& color, int amount)
{
#ifdef SDL_VIDEO_OPENGL
	if (amount > MAX_COLOR)
		amount = MAX_COLOR;
	for (int i = 0; i < amount; ++i) {
		for (int c = 0; c < 4; ++c) {
			_colorArray[i * 4 + c] = color[c];
		}
	}
	glColorPointer(4, GL_FLOAT, 0, _colorArray);
#endif
}

void GL1Frontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
{
#ifdef SDL_VIDEO_OPENGL
	if (texture == nullptr || !texture->isValid())
		return;

	_color[3] = alpha;
	setColorPointer(_color, 4);

	const TextureCoords texCoords(texture);

	getTrimmed(texture, x, y, w, h);

	const float x1 = x * _rx;
	const float y1 = y * _ry;
	const float nw = w * _rx;
	const float nh = h * _ry;
	const float centerx = nw / 2.0f;
	const float centery = nh / 2.0f;
	const float minx = -centerx;
	const float maxx = centerx;
	const float miny = -centery;
	const float maxy = centery;
	vertices[0] = minx;
	vertices[1] = miny;
	vertices[2] = maxx;
	vertices[3] = miny;
	vertices[4] = maxx;
	vertices[5] = maxy;
	vertices[6] = minx;
	vertices[7] = maxy;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef((GLfloat) (x1 + centerx), (GLfloat) (y1 + centery), (GLfloat) 0.0f);
	glRotated(angle, (GLdouble) 0.0, (GLdouble) 0.0, (GLdouble) 1.0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords.texCoords);
	bindTexture(texture, 0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
	GL_checkError();
#endif
}

void GL1Frontend::renderRect (int x, int y, int w, int h, const Color& color)
{
#ifdef SDL_VIDEO_OPENGL
	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();

	const float nx = x * _rx;
	const float ny = y * _ry;
	const float nw = w * _rx;
	const float nh = h * _ry;
	vertices[0] = nx;
	vertices[1] = ny;
	vertices[2] = nx + nw;
	vertices[3] = ny;
	vertices[4] = nx + nw;
	vertices[5] = ny + nh;
	vertices[6] = nx;
	vertices[7] = ny + nh;

	setColorPointer(color, 4);
	enableTextureUnit(*_currentTextureUnit, false);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	GL_checkError();
	enableTextureUnit(*_currentTextureUnit, true);
#endif
}

int GL1Frontend::renderFilledPolygon (int *vx, int *vy, int n, const Color& color)
{
#ifdef SDL_VIDEO_OPENGL
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;
	if (n >= sizeof(vertices))
		return -1;
	setColorPointer(color, 4);
	enableTextureUnit(*_currentTextureUnit, false);
	int index = 0;
	for (int i = 0; i < n; ++i) {
		vertices[index++] = vx[i] * _rx;
		vertices[index++] = vy[i] * _ry;
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, n);
	GL_checkError();
	enableTextureUnit(*_currentTextureUnit, true);
	return 0;
#else
	return -1;
#endif
}

int GL1Frontend::renderPolygon (int *vx, int *vy, int n, const Color& color)
{
#ifdef SDL_VIDEO_OPENGL
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;
	if (n >= sizeof(vertices))
		return -1;
	setColorPointer(color, 4);
	enableTextureUnit(*_currentTextureUnit, false);
	int index = 0;
	for (int i = 0; i < n; ++i) {
		vertices[index++] = vx[i] * _rx;
		vertices[index++] = vy[i] * _ry;
	}
	glDrawArrays(GL_LINE_LOOP, 0, n);
	GL_checkError();
	enableTextureUnit(*_currentTextureUnit, true);
	return 0;
#else
	return -1;
#endif
}

void GL1Frontend::renderFilledRect (int x, int y, int w, int h, const Color& fillColor)
{
#ifdef SDL_VIDEO_OPENGL
	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();

	const float nx = x * _rx;
	const float ny = y * _ry;
	const float nw = w * _rx;
	const float nh = h * _ry;

	setColorPointer(fillColor, 4);
	enableTextureUnit(*_currentTextureUnit, false);
	const float minx = nx;
	const float maxx = nx + nw;
	const float miny = ny;
	const float maxy = ny + nh;
	vertices[0] = minx;
	vertices[1] = miny;
	vertices[2] = maxx;
	vertices[3] = miny;
	vertices[4] = maxx;
	vertices[5] = maxy;
	vertices[6] = minx;
	vertices[7] = maxy;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	GL_checkError();
	enableTextureUnit(*_currentTextureUnit, true);
#endif
}

void GL1Frontend::destroyTexture (TextureData *data)
{
#ifdef SDL_VIDEO_OPENGL
	const TexNum texnum = GL1getTexNum(data);
	glDeleteTextures(1, &texnum);
	GL_checkError();
#endif
}

bool GL1Frontend::loadTexture (Texture *texture, const std::string& filename)
{
#ifdef SDL_VIDEO_OPENGL
	const std::string file = FS.getFileFromURL("pics://" + filename + ".png")->getName();
	SDL_RWops *src = FS.createRWops(file);
	if (src == nullptr) {
		Log::error(LOG_CLIENT, "could not load the file: %s", file.c_str());
		return false;
	}
	SDL_Surface *surface = IMG_Load_RW(src, 1);
	if (!surface) {
		sdlCheckError();
		Log::error(LOG_CLIENT, "could not load the texture: %s", file.c_str());
		return false;
	}

	if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
		SDL_PixelFormat *destFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
		if (destFormat == nullptr) {
			SDL_FreeSurface(surface);
			return false;
		}
		SDL_Surface* temp = SDL_ConvertSurface(surface, destFormat, 0);
		SDL_FreeFormat(destFormat);
		if (temp == nullptr) {
			SDL_FreeSurface(surface);
			return false;
		}
		SDL_FreeSurface(surface);
		surface = temp;
	}

	GLuint texnum;
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	unsigned char* pixels = static_cast<unsigned char*>(surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_checkError();
	texture->setData(reinterpret_cast<TextureData*>(texnum));
	texture->setRect(0, 0, surface->w, surface->h);
	SDL_FreeSurface(surface);
	return texnum != 0;
#else
	return false;
#endif
}

void GL1Frontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
#ifdef SDL_VIDEO_OPENGL
	const float nx1 = x1 * _rx;
	const float ny1 = y1 * _ry;
	const float nx2 = x2 * _rx;
	const float ny2 = y2 * _ry;
	vertices[0] = nx1;
	vertices[1] = ny1;
	vertices[2] = nx2;
	vertices[3] = ny2;

	glEnable(GL_LINE_SMOOTH);
	setColorPointer(color, 4);
	enableTextureUnit(*_currentTextureUnit, false);
	glDrawArrays(GL_LINES, 0, 2);
	glDisable(GL_LINE_SMOOTH);
	GL_checkError();
	enableTextureUnit(*_currentTextureUnit, true);
#endif
}

void GL1Frontend::makeScreenshot (const std::string& filename)
{
#ifdef SDL_VIDEO_OPENGL
#ifndef EMSCRIPTEN
	const int bytesPerPixel = 3;
	std::unique_ptr<GLubyte> pixels(new GLubyte[bytesPerPixel * _width * _height]);
	int rowPack;
	glGetIntegerv(GL_PACK_ALIGNMENT, &rowPack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	glPixelStorei(GL_PACK_ALIGNMENT, rowPack);

	std::unique_ptr<SDL_Surface> surface(SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			0x000000ff, 0x0000ff00, 0x00ff0000
#else
			0x00ff0000, 0x0000ff00, 0x000000ff
#endif
			, 0));
	if (!surface)
		return;
	const int pitch = _width * bytesPerPixel;
	for (int y = 0; y < _height; ++y)
		memcpy((uint8 *) surface->pixels + surface->pitch * y, (uint8 *) pixels.get() + pitch * (_height - y - 1), pitch);
	const std::string fullFilename = FS.getAbsoluteWritePath() + filename + "-" + dateutil::getDateString() + ".png";
	IMG_SavePNG(surface.get(), fullFilename.c_str());
#endif
#endif
}

void GL1Frontend::updateViewport (int x, int y, int width, int height)
{
#ifdef SDL_VIDEO_OPENGL
	const float wantAspect = (float)width / height;
	const float realAspect = (float)_width / _height;

	float scale;
	if (fequals(wantAspect, realAspect)) {
		/* The aspect ratios are the same, just scale appropriately */
		scale = (float) _width / width;
		_viewPort.x = 0;
		_viewPort.w = _width;
		_viewPort.h = _height;
		_viewPort.y = 0;
	} else if (wantAspect > realAspect) {
		/* We want a wider aspect ratio than is available - letterbox it */
		scale = (float) _width / width;
		_viewPort.x = 0;
		_viewPort.w = _width;
		_viewPort.h = (int) SDL_ceil(height * scale);
		_viewPort.y = (_height - _viewPort.h) / 2;
	} else {
		/* We want a narrower aspect ratio than is available - use side-bars */
		scale = (float) _height / height;
		_viewPort.y = 0;
		_viewPort.h = _height;
		_viewPort.w = (int) SDL_ceil(width * scale);
		_viewPort.x = (_width - _viewPort.w) / 2;
	}

	_rx = scale;
	_ry = scale;
	glViewport(_viewPort.x, _viewPort.y, _viewPort.w, _viewPort.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const GLdouble _w = static_cast<GLdouble>(_viewPort.w);
	const GLdouble _h = static_cast<GLdouble>(_viewPort.h);
	glOrtho(0.0, _w, _h, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	GL_checkError();
#endif
}

void GL1Frontend::renderBegin ()
{
#ifdef SDL_VIDEO_OPENGL
	SDL_GL_MakeCurrent(_window, _context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void GL1Frontend::renderEnd ()
{
#ifdef SDL_VIDEO_OPENGL
	SDL_GL_SwapWindow(_window);
	GL_checkError();
#endif
}
