/*  Original coding tutorial and assets from: https://learnopengl.com/In-Practice/2D-Game/Particles
	Coding srtuctures inspired from: https://learnopengl.com/code_viewer.php?code=in-practice/breakout/particle_generator
	Modified by Qixu Feng */

#include "Particles.h"

GenerateParticle::GenerateParticle(Shader* inShader, Texture* inTexture, int inNum)
	: shader(inShader), texture(inTexture), num(inNum) { this->init(); 
}

void GenerateParticle::Update(float dt, GameObject* otherObject, int newParticle, Vector2 offset) {
	for (int i = 0; i < newParticle; i++) {

	}
}

void GenerateParticle::init() {
	GLuint VBO;
	float particleMatrix[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleMatrix), particleMatrix, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	for (int i = 0; i < this->num; ++i) {
		this->particles.push_back(Particle());
	}
}