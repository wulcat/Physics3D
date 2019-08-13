#pragma once

#include "bindable.h"

#include "../engine/math/vec.h"

#include <string>

class Texture : public Bindable {
public:
	int unit;
	unsigned int width;
	unsigned int height;
	
	int format;
	int channels;

	Texture(unsigned int width, unsigned int height, const void* buffer, int format);
	Texture(unsigned int width, unsigned int height);

	void bind(int unit);
	void bind() override;
	void unbind() override;
	void close() override;

	void resize(unsigned int width, unsigned int height);
	void resize(unsigned int width, unsigned int height, const void* buffer);

	void loadFrameBufferTexture(unsigned int width, unsigned int height);

	Texture* colored(Vec3 color);
	Texture* colored(Vec4 color);
};

Texture* load(const std::string& name);

class HDRTexture : public Bindable {
public:
	int unit;
	unsigned int width;
	unsigned int height;

	HDRTexture(unsigned int width, unsigned int height, const void* buffer);
	HDRTexture(unsigned int width, unsigned int height);

	void bind(int unit);
	void bind() override;
	void unbind() override;
	void close() override;

	void resize(unsigned int width, unsigned int height);
	void resize(unsigned int width, unsigned int height, const void* buffer);
};

class MultisampleTexture : public Bindable {
public:
	int unit;
	unsigned int width;
	unsigned int height;
	unsigned int samples;

	MultisampleTexture(unsigned int width, unsigned int height, unsigned int samples);

	void bind(int unit);
	void bind() override;
	void unbind() override;
	void close() override;

	void resize(unsigned int width, unsigned int height);
};

class CubeMap : public Bindable {
public:
	int unit;

	CubeMap(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back);

	void load(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back);

	void bind(int unit);
	void bind() override;
	void unbind() override;
	void close() override;
};

class DepthTexture : public Bindable {
public:
	int unit;
	unsigned int width;
	unsigned int height;

	DepthTexture(unsigned int width, unsigned int height);

	void bind(int unit);
	void bind() override;
	void unbind() override;
	void close() override;
};
