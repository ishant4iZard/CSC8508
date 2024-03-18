/*  Original coding tutorial and assets from: https://learnopengl.com/In-Practice/2D-Game/Particles
	Coding srtuctures inspired from: https://learnopengl.com/code_viewer.php?code=in-practice/breakout/particle_generator.h
	Modified by Qixu Feng */

#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"
#include "Event.h"
#include "GameWorld.h"
#include "GameObject.h"
#include "RenderObject.h"

namespace NCL {
	namespace CSC8503 {
        class Particle {
        public:
            Particle() : position(0.0f, 0.0f, 0.0f), velocity(0.0f, 0.0f), color(1.0f, 1.0f, 1.0f, 1.0f), life(0.0f) {}
            ~Particle() {};

            void SetPosition(const Vector3& inPosition) { position = inPosition; }
            Vector3 Getposition() const { return position; }

            void SetVelocity(const Vector2& inVelocity) { velocity = inVelocity; }
            Vector2 GetVelocity() const { return velocity; }

            void SetColor(const Vector4& inColor) { color = inColor; }
            Vector4 GetColor() const { return color; }

            void SetLife(const float inLife) { life = inLife; }
            float GetLife() const { return life; }


        protected:
            Vector3 position;
            Vector2 velocity;
            Vector4 color;

            float life;

        };

        class GenerateParticle : public OGLRenderer {
        public:
            GenerateParticle(OGLShader* inShader, OGLTexture* inTexture, int inNum);

            void Update(float dt, GameObject* otherObject, int newParticle, Vector2 offset = Vector2(0.0f, 0.0f));
            void Draw();

        protected:
            void init();
            void respawnParticle(Particle* inParticle, GameObject* otherObject, Vector2 offset = Vector2(0.0f, 0.0f));
            GLuint FirstUnusedParticle();

            GLuint VAO;
            GLuint	particleTex;
            OGLShader* shader;
            OGLTexture* texture;
            
            std::vector<Particle> particles;
            int num;
        };
	}
}