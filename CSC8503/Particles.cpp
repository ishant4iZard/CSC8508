/*  Original coding tutorial and assets from: https://learnopengl.com/In-Practice/2D-Game/Particles
	Coding srtuctures inspired from: https://learnopengl.com/code_viewer.php?code=in-practice/breakout/particle_generator
	Modified by Qixu Feng */

#include "Particles.h"
#include "Projectile.h"


GenerateParticle::GenerateParticle(OGLShader* inShader, OGLTexture* inTexture, int inNum)
	: OGLRenderer(*Window::GetWindow()), shader(inShader), texture(inTexture), num(inNum) { this->init(); }

void GenerateParticle::Update(float dt, Projectile* otherObject, int newParticle, Vector3 offset) {
	for (int i = 0; i < newParticle; ++i) {
		int spareParticle = this->FirstUnusedParticle();
		this->respawnParticle(&(this->particles[spareParticle]), otherObject, offset);
	}
	for (int i = 0; i < this->num; ++i) {
		Particle& p = this->particles[i];
		p.SetLife(p.GetLife() - dt);
		if (p.GetLife() > 0.0f) {
			Vector4 tempColor = p.GetColor();
			p.SetPosition(p.Getposition() - p.GetVelocity() * dt);
			p.SetColor(Vector4(tempColor.x, tempColor.y, tempColor.z, 
				tempColor.w - dt * 2.5f));
		}
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

void GenerateParticle::Draw() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	BindShader(*shader);
	glGenTextures(1, &particleTex);
	for (Particle particle : this->particles) {
		if (particle.GetLife() > 0.0f) {
			Vector3 tempParticle = particle.Getposition();
			Vector4 tempColor = particle.GetColor();
			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "offset"), 0, (float*)&tempParticle);
			glUniform4fv(glGetUniformLocation(shader->GetProgramID(), "color"), 0, (float*)&tempColor);
			glBindTexture(GL_TEXTURE_2D, particleTex);
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLuint GenerateParticle::FirstUnusedParticle() {
	int lastUsedParticle = 0;
	for (int i = lastUsedParticle; i < this->num; ++i) {
		if (this->particles[i].GetLife() <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	for (int i = 0; i < lastUsedParticle; ++i) {
		if (this->particles[i].GetLife() < 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	return 0;
}

void GenerateParticle::respawnParticle(Particle* inParticle, Projectile* otherObject, Vector3 offset) {
	float random = ((rand() % 100) - 50) / 10.0f;
	float randColor = 0.5 + ((rand() % 100) / 100.0f);

	inParticle->SetPosition(otherObject->GetTransform().GetPosition() + Vector3(random, random, random) + offset);
	inParticle->SetColor(Vector4(randColor, randColor, randColor, 1.0f));
	inParticle->SetLife(10.0f);
	inParticle->SetVelocity(Vector2(1.0f, 1.0f) * 0.1f); //* otherObject.GetVelocity());
}
